# Smart Lock Backend API

スマートロックシステムのバックエンドAPI。ユーザー認証、部屋の予約管理、鍵の配布、BLE UUID取得機能を提供します。

## 技術スタック

- **言語**: Go 1.24
- **フレームワーク**: net/http (標準ライブラリ)
- **ORM**: GORM v1.31.1
- **データベース**: PostgreSQL 16
- **認証**: JWT (github.com/golang-jwt/jwt/v5)
- **パスワードハッシュ**: bcrypt
- **デプロイ**: Kubernetes (minikube)
- **CI/CD**: GitHub Actions + ArgoCD

## アーキテクチャ

クリーンアーキテクチャを採用:

```
app/
├── domain/          # ビジネスロジック層
│   ├── entity/      # エンティティ定義
│   └── repository/  # リポジトリインターフェース
├── infrastructure/  # インフラ層
│   ├── database/    # データベース接続
│   └── persistence/ # リポジトリ実装
├── interface/       # インターフェース層
│   ├── handler/     # HTTPハンドラ
│   ├── middleware/  # 認証ミドルウェア
│   └── presenter/   # レスポンス整形
├── usecase/         # ユースケース層
└── pkg/utils/       # ユーティリティ
```

## 環境変数

| 変数名           | 説明                   | デフォルト値                |
| ---------------- | ---------------------- | --------------------------- |
| `DB_HOST`        | PostgreSQLホスト       | `localhost`                 |
| `DB_PORT`        | PostgreSQLポート       | `5432`                      |
| `DB_NAME`        | データベース名         | `smart_lock_db`             |
| `DB_USER`        | データベースユーザー   | `postgres`                  |
| `DB_PASSWORD`    | データベースパスワード | -                           |
| `KEY_SERVER_URL` | 鍵サーバーURL          | `http://192.168.11.24:8081` |

## API エンドポイント

### 認証系 (Authentication)

#### POST `/api/register`

ユーザー登録

**リクエスト:**

```json
{
	"email": "user@example.com",
	"password": "password123",
	"name": "User Name"
}
```

**レスポンス:**

```json
{
	"token": "eyJhbGciOiJIUzI1NiIs...",
	"user": {
		"id": 1,
		"email": "user@example.com",
		"name": "User Name"
	}
}
```

#### POST `/api/login`

ログイン

**リクエスト:**

```json
{
	"email": "user@example.com",
	"password": "password123"
}
```

**レスポンス:**

```json
{
	"token": "eyJhbGciOiJIUzI1NiIs...",
	"user": {
		"id": 1,
		"email": "user@example.com",
		"name": "User Name"
	}
}
```

#### GET `/api/me`

現在のユーザー情報取得（要認証）

**ヘッダー:**

```
Authorization: Bearer <token>
```

**レスポンス:**

```json
{
	"user": {
		"id": 1,
		"email": "user@example.com",
		"name": "User Name"
	}
}
```

### 部屋管理 (Room Management)

#### GET `/api/rooms`

全部屋一覧取得

**レスポンス:**

```json
{
	"rooms": [
		{
			"id": 101,
			"room_name": "101",
			"created_at": "2026-02-04T12:00:00Z"
		},
		{
			"id": 102,
			"room_name": "102",
			"created_at": "2026-02-04T12:00:00Z"
		}
	]
}
```

#### POST `/api/book-room`

部屋予約（要認証）

**ヘッダー:**

```
Authorization: Bearer <token>
```

**リクエスト:**

```json
{
	"room_id": 101
}
```

**レスポンス:**

```json
{
	"message": "Room booked successfully"
}
```

#### GET `/api/my-bookings`

自分の予約一覧取得（要認証）

**ヘッダー:**

```
Authorization: Bearer <token>
```

**レスポンス:**

```json
{
	"bookings": [
		{
			"id": 1,
			"room_id": 101,
			"user_id": 1,
			"created_at": "2026-02-04T12:00:00Z"
		}
	]
}
```

#### DELETE `/api/cancel-booking`

予約キャンセル（要認証）

**ヘッダー:**

```
Authorization: Bearer <token>
```

**リクエスト:**

```json
{
	"booking_id": 1
}
```

**レスポンス:**

```json
{
	"message": "Booking cancelled successfully"
}
```

### 鍵管理 (Key Management)

#### GET `/api/rooms/keys`

予約した部屋の公開鍵取得（要認証）

**ヘッダー:**

```
Authorization: Bearer <token>
```

**レスポンス:**

```json
{
	"keys": [
		{
			"room_id": 101,
			"room_name": "101",
			"public_key": "-----BEGIN ED25519 PUBLIC KEY-----\n...\n-----END ED25519 PUBLIC KEY-----\n",
			"booking_id": 1
		}
	]
}
```

外部鍵サーバー（`KEY_SERVER_URL`）からEd25519公開鍵を取得します。

### スマートロック連携 (Smart Lock Integration)

#### GET `/api/ble-uuid?room_id=101`

部屋のBLE UUID取得（認証不要）

**パラメータ:**

- `room_id`: 部屋ID (必須)

**レスポンス:**

```json
{
	"ble_uuids": ["BLE-UUID-101"]
}
```

### ヘルスチェック (Health Check)

#### GET `/health`

アプリケーション稼働確認

**レスポンス:**

```
nice
```

## ローカル開発

### 前提条件

- Go 1.24以上
- PostgreSQL 16
- Docker (オプション)

### セットアップ

1. リポジトリのクローン

```bash
git clone https://github.com/yourusername/smart-lock-backend.git
cd smart-lock-backend
```

2. 環境変数の設定

```bash
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=smart_lock_db
export DB_USER=postgres
export DB_PASSWORD=your_password
export KEY_SERVER_URL=http://192.168.11.24:8081
```

3. 依存関係のインストール

```bash
cd app
go mod download
```

4. アプリケーションの起動

```bash
go run main.go
```

サーバーが `http://localhost:8080` で起動します。

## Kubernetes デプロイ

### 前提条件

- Kubernetes クラスタ (minikube推奨)
- kubectl
- ArgoCD (GitOps用)

### デプロイ手順

1. Kubernetesマニフェストリポジトリをクローン

```bash
git clone https://github.com/yourusername/smart-lock-manifests.git
```

2. PostgreSQLデプロイ

```bash
kubectl apply -f k8s/postgres-pvc.yaml
kubectl apply -f k8s/postgres-deployment.yaml
kubectl apply -f k8s/postgres-service.yaml
```

3. アプリケーションデプロイ

```bash
kubectl apply -f k8s/configmap.yaml
kubectl apply -f k8s/secret.yaml
kubectl apply -f k8s/deployment.yaml
kubectl apply -f k8s/service.yaml
```

4. Pod確認

```bash
kubectl get pods -n smart-lock
```

## データベーススキーマ

### users テーブル

| カラム名   | 型        | 説明                       |
| ---------- | --------- | -------------------------- |
| id         | SERIAL    | ユーザーID (主キー)        |
| email      | VARCHAR   | メールアドレス (ユニーク)  |
| password   | VARCHAR   | bcryptハッシュ化パスワード |
| name       | VARCHAR   | ユーザー名                 |
| created_at | TIMESTAMP | 作成日時                   |
| updated_at | TIMESTAMP | 更新日時                   |
| deleted_at | TIMESTAMP | 削除日時 (ソフトデリート)  |

### rooms テーブル

| カラム名   | 型        | 説明            |
| ---------- | --------- | --------------- |
| id         | SERIAL    | 部屋ID (主キー) |
| room_name  | VARCHAR   | 部屋名          |
| created_at | TIMESTAMP | 作成日時        |
| updated_at | TIMESTAMP | 更新日時        |
| deleted_at | TIMESTAMP | 削除日時        |

初期データ: ID 101, 102, 103 が自動作成されます。

### user_rooms テーブル

| カラム名   | 型        | 説明                  |
| ---------- | --------- | --------------------- |
| id         | SERIAL    | 予約ID (主キー)       |
| user_id    | INTEGER   | ユーザーID (外部キー) |
| room_id    | INTEGER   | 部屋ID (外部キー)     |
| created_at | TIMESTAMP | 予約日時              |
| updated_at | TIMESTAMP | 更新日時              |
| deleted_at | TIMESTAMP | 削除日時              |

## CI/CD

GitHub ActionsとArgoCDを使用したGitOpsワークフロー:

1. `main`ブランチにプッシュ
2. GitHub ActionsがDockerイメージをビルド
3. マニフェストリポジトリのイメージタグを更新
4. ArgoCDが変更を検知して自動デプロイ

## トラブルシューティング

### Pod が CrashLoopBackOff になる

1. ログを確認:

```bash
kubectl logs <pod-name> -n smart-lock
```

2. データベース接続を確認:

```bash
kubectl get configmap smart-lock-api-config -n smart-lock -o yaml
```

3. Secretを確認:

```bash
kubectl get secret smart-lock-api-secret -n smart-lock -o yaml
```

### データベースをリセットしたい

PostgreSQL Podに接続:

```bash
kubectl exec -it <postgres-pod-name> -n smart-lock -- psql -U postgres -d smart_lock_db
```

テーブル削除:

```sql
DROP TABLE user_rooms;
DROP TABLE rooms;
DROP TABLE users;
```

Podを再起動してマイグレーション実行:

```bash
kubectl rollout restart deployment smart-lock-api -n smart-lock
```

## ライセンス

MIT License

## 作成者

Your Name
