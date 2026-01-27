package middleware

import (
	"context"
	"net/http"
	"strings"

	"smart_lock_back/pkg/utils"
)

// AuthMiddleware - JWT認証ミドルウェア
//
// ここに認証トークンの検証処理を実装する
//
// AuthMiddleware関数:
//  1. Authorizationヘッダーを取得
//  2. ヘッダーが空の場合は401 Unauthorizedエラー
//  3. "Bearer <token>"形式かチェック
//  4. 形式が正しくない場合は401エラー
//  5. JWTユーティリティでトークンを検証
//  6. トークンが無効な場合は401エラー
//  7. トークンからuser_idとemailを取得
//  8. コンテキストにuser_idとemailを設定
//  9. 次のハンドラーを呼び出し
//
// このミドルウェアを適用したエンドポイントは認証が必要になる
//
// 使用例:
//
//	http.HandleFunc("/api/me", middleware.AuthMiddleware(handler.Me))
func AuthMiddleware(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		authHeader := r.Header.Get("Authorization")
		if authHeader == "" {
			http.Error(w, "Unauthorized", http.StatusUnauthorized)
			return
		}

		parts := strings.Split(authHeader, " ")
		if len(parts) != 2 || parts[0] != "Bearer" {
			http.Error(w, "Unauthorized", http.StatusUnauthorized)
			return
		}

		token := parts[1]
		claims, err := utils.ValidateToken(token)
		if err != nil {
			http.Error(w, "Unauthorized", http.StatusUnauthorized)
			return
		}

		ctx := context.WithValue(r.Context(), "user_id", claims.UserID)
		ctx = context.WithValue(ctx, "email", claims.Email)
		next.ServeHTTP(w, r.WithContext(ctx))
	}
}
