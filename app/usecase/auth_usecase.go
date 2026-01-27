package usecase

import (
	"smart_lock_back/domain/entity"
	"smart_lock_back/domain/repository"
	"smart_lock_back/pkg/utils"
)

type AuthUsecase struct {
	userRepo repository.UserRepository
}

// NewAuthUsecase - AuthUsecaseのインスタンスを生成
func NewAuthUsecase(userRepo repository.UserRepository) *AuthUsecase {
	return &AuthUsecase{userRepo: userRepo}
}

// Register - 新規ユーザー登録
func (u *AuthUsecase) Register(email, password, name string) (string, *entity.User, error) {
	user := &entity.User{
		Email:    email,
		Password: password,
		Name:     name,
	}

	// バリデーション
	if err := user.Validate(); err != nil {
		return "", nil, err
	}

	// パスワードのハッシュ化
	if err := user.HashPassword(); err != nil {
		return "", nil, err
	}

	// ユーザーをDBに保存
	if err := u.userRepo.Create(user); err != nil {
		return "", nil, err
	}

	// JWTトークンを生成
	token, err := utils.GenerateToken(user.ID, user.Email)
	if err != nil {
		return "", nil, err
	}

	return token, user, nil
}

// Login - ユーザーログイン
func (u *AuthUsecase) Login(email, password string) (string, *entity.User, error) {
	// ユーザーを検索
	user, err := u.userRepo.FindByEmail(email)
	if err != nil {
		return "", nil, err
	}

	// パスワードの検証
	if err := user.CheckPassword(password); err != nil {
		return "", nil, err
	}

	// JWTトークンを生成
	token, err := utils.GenerateToken(user.ID, user.Email)
	if err != nil {
		return "", nil, err
	}

	return token, user, nil
}

// GetUserByID - ユーザーIDでユーザー情報を取得
func (u *AuthUsecase) GetUserByID(id uint) (*entity.User, error) {
	return u.userRepo.FindByID(id)
}
