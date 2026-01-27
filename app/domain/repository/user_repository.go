package repository

import "smart_lock_back/domain/entity"

// ※このインターフェースの実装はinfrastructure/persistence層で行う
type UserRepository interface {
	// Create - 新規ユーザーを登録する
	Create(user *entity.User) error

	// FindByEmail - メールアドレスでユーザーを検索する(ログイン時に使用)
	FindByEmail(email string) (*entity.User, error)

	// FindByID - IDでユーザーを検索する(認証済みユーザー情報取得時に使用)
	FindByID(id uint) (*entity.User, error)
}
