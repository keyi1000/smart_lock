package utils

import (
	"os"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

type Claims struct {
	UserID               uint   `json:"user_id"` // ユーザーID
	Email                string `json:"email"`   // メールアドレス
	jwt.RegisteredClaims        // 標準クレーム（有効期限、発行日時など）
}

var jwtSecret []byte

func init() {
	// 環境変数JWT_SECRETを取得
	secret := os.Getenv("JWT_SECRET")

	// 空の場合はデフォルト値を設定
	if secret == "" {
		secret = "default_secret_key" // 開発環境用のデフォルト値
	}

	// グローバル変数に保存
	jwtSecret = []byte(secret)
}

func GenerateToken(userID uint, email string) (string, error) {
	// Claims構造体を作成
	claims := &Claims{
		UserID: userID,
		Email:  email,
		RegisteredClaims: jwt.RegisteredClaims{
			// 有効期限を7日間後に設定
			ExpiresAt: jwt.NewNumericDate(time.Now().Add(7 * 24 * time.Hour)),
			IssuedAt:  jwt.NewNumericDate(time.Now()),
		},
	}

	// トークンを作成
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)

	// トークンに署名して文字列を生成
	tokenString, err := token.SignedString(jwtSecret)
	if err != nil {
		return "", err
	}

	// トークン文字列を返却
	return tokenString, nil
}

func ValidateToken(tokenString string) (*Claims, error) {
	// トークンをパース
	token, err := jwt.ParseWithClaims(tokenString, &Claims{}, func(token *jwt.Token) (interface{}, error) {
		// 署名方式がHMACかチェック
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, jwt.ErrSignatureInvalid
		}
		return jwtSecret, nil
	})

	if err != nil {
		return nil, err
	}

	// クレームを取得
	claims, ok := token.Claims.(*Claims)
	if !ok || !token.Valid {
		return nil, jwt.ErrTokenMalformed
	}

	// クレームを返却
	return claims, nil
}
