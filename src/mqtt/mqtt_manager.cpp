// mqtt_manager.cpp
// 日本語コメントを追加したMQTTマネージャーの実装
#include "mqtt/mqtt_manager.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// 内部実装用の構造体（Pimplパターン風）
struct MqttManager::Impl {
    // ネットワーククライアント（PubSubClient が使用）
    WiFiClient espClient;
    // MQTT クライアントライブラリのインスタンス
    PubSubClient client;
    // ブローカーのホスト名（設定から渡される）
    const char* broker = nullptr;
    // ブローカーのポート（デフォルト1883）
    uint16_t port = 1883;
    // クライアントID（文字列）
    String clientId;
    // ユーザーが設定するメッセージハンドラ（受信コールバック）
    std::function<void(const char*, const uint8_t*, unsigned int)> userHandler;
    // シングルトン風にこのインスタンスへのポインタを保持（静的コールバックから参照するため）
    static Impl* instance;

    // コンストラクタ: PubSubClient の初期化とインスタンス登録
    Impl(): client(espClient) {
        instance = this;
    }

    // PubSubClient のコールバックに使う静的関数
    // 受信したメッセージを userHandler に委譲するか、なければシリアルに出力する
    static void staticCallback(char* topic, uint8_t* payload, unsigned int length) {
        if (!instance) return; // インスタンス未初期化なら無視
        if (instance->userHandler) {
            // ユーザー定義のハンドラに処理を委譲
            instance->userHandler(topic, payload, length);
        } else {
            // デフォルト処理: シリアルにトピックとペイロードを表示
            Serial.print("MQTT msg[");
            Serial.print(topic);
            Serial.print("] ");
            for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
            Serial.println();
        }
    }

    // MQTT ブローカーへ接続を試みる。接続済みなら何もしない。
    // 接続成功後、デフォルトの購読トピックを購読する。
    void reconnect() {
        if (client.connected()) return; // 既に接続済み
        if (!broker) return; // ブローカー未設定なら何もしない
        Serial.print("Attempting MQTT connection...");
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            // 設定ファイルの購読トピックを購読（存在する前提）
            client.subscribe(MQTT_TOPIC_SUB);
            Serial.print("Subscribed to "); Serial.println(MQTT_TOPIC_SUB);
        } else {
            // 失敗したら状態コードを出力して少し待って再試行
            Serial.print("failed, rc="); Serial.print(client.state()); Serial.println(" try again in 5s");
            delay(5000);
        }
    }
};

// 静的メンバの初期化
MqttManager::Impl* MqttManager::Impl::instance = nullptr;

// MqttManager のコンストラクタ: 内部 Impl を生成
MqttManager::MqttManager() {
    impl = new Impl();
}

// begin: ブローカー情報を設定し、即座に接続を試みる
void MqttManager::begin(const char* broker, uint16_t port, const char* clientId) {
    impl->broker = broker;
    impl->port = port;
    // clientId が渡されなければ config.h の定義を使用
    impl->clientId = clientId ? String(clientId) : String(MQTT_CLIENT_ID);
    impl->client.setServer(broker, port);
    // 受信時のコールバックを設定（静的関数を登録）
    impl->client.setCallback(Impl::staticCallback);
    // 即時接続を試みる（失敗しても loop() 内で再試行される）
    impl->reconnect();
}

// loop: WiFi 接続があるときに MQTT のループ処理を呼ぶ
// 接続が切れていれば reconnect() を試みる
void MqttManager::loop() {
    if (!WiFi.isConnected()) return; // WiFi 未接続なら何もしない
    if (!impl->client.connected()) {
        impl->reconnect();
    } else {
        // MQTT ライブラリの内部ループを呼ぶ（受信処理など）
        impl->client.loop();
    }
}

// publish: トピックにペイロードを送信。接続されていない場合は false を返す
bool MqttManager::publish(const char* topic, const char* payload) {
    if (!impl->client.connected()) return false;
    return impl->client.publish(topic, payload);
}

// subscribe: 指定トピックを購読する（接続必須）
bool MqttManager::subscribe(const char* topic) {
    if (!impl->client.connected()) return false;
    return impl->client.subscribe(topic);
}

// ユーザー側で受信ハンドラを設定するためのメソッド
void MqttManager::setMessageHandler(std::function<void(const char*, const uint8_t*, unsigned int)> handler) {
    impl->userHandler = handler;
}
