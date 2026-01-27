package persistence

import (
	"smart_lock_back/domain/entity"
	"smart_lock_back/domain/repository"

	"gorm.io/gorm"
)

// UserRepositoryImpl - UserRepositoryインターフェースの実装

type UserRepositoryImpl struct {
	db *gorm.DB
}

// NewUserRepository - UserRepositoryImplのインスタンスを生成して返す
func NewUserRepository(db *gorm.DB) repository.UserRepository {
	return &UserRepositoryImpl{db: db}
}

// Create - ユーザーをDBに保存
func (r *UserRepositoryImpl) Create(user *entity.User) error {
	return r.db.Create(user).Error
}

// FindByEmail - メールアドレスでユーザーを検索
func (r *UserRepositoryImpl) FindByEmail(email string) (*entity.User, error) {
	var user entity.User
	if err := r.db.Where("email = ?", email).First(&user).Error; err != nil {
		return nil, err
	}
	return &user, nil
}

// FindByID - IDでユーザーを検索
func (r *UserRepositoryImpl) FindByID(id uint) (*entity.User, error) {
	var user entity.User
	if err := r.db.First(&user, id).Error; err != nil {
		return nil, err
	}
	return &user, nil
}
