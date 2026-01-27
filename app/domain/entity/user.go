package entity

import (
	"errors"
	"time"

	"golang.org/x/crypto/bcrypt"
)

type User struct {
	ID        uint      `json:"id"`
	Email     string    `json:"email"`
	Password  string    `json:"-"` // JSONに含めない
	Name      string    `json:"name"`
	CreatedAt time.Time `json:"created_at"`
	UpdatedAt time.Time `json:"updated_at"`
	DeletedAt *time.Time `json:"deleted_at,omitempty"`
}

// Validate - ユーザーデータのバリデーション
func (u *User) Validate() error {
	if u.Email == "" {
		return errors.New("メールアドレスが存在しません")
	}
	if u.Password == "" {
		return errors.New("パスワードが存在しません")
	}
	if len(u.Password) < 6 {
		return errors.New("パスワードは6文字以上である必要があります")
	}
	return nil
}

// HashPassword - パスワードをハッシュ化
func (u *User) HashPassword() error {
	hashedPassword, err := bcrypt.GenerateFromPassword([]byte(u.Password), bcrypt.DefaultCost)
	if err != nil {
		return err
	}
	u.Password = string(hashedPassword)
	return nil
}

// CheckPassword - パスワードを検証
func (u *User) CheckPassword(password string) error {
	return bcrypt.CompareHashAndPassword([]byte(u.Password), []byte(password))
}
