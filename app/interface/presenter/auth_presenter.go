package presenter

import (
	"smart_lock_back/domain/entity"
	"time"
)

// AuthPresenter defines the interface for formatting authentication responses.
type AuthPresenter interface {
	ToAuthResponse(user *entity.User, token string) AuthResponse
	ToUserResponse(user *entity.User) UserResponse
	ToErrorResponse(err string) ErrorResponse
}

// authPresenterImpl is the implementation of AuthPresenter.
type authPresenterImpl struct{}

// NewAuthPresenter creates a new instance of AuthPresenter.
func NewAuthPresenter() AuthPresenter {
	return &authPresenterImpl{}
}

// AuthResponse represents the structure of the authentication response
// containing the JWT token and user details.
type AuthResponse struct {
	Token string       `json:"token"`
	User  UserResponse `json:"user"`
}

// UserResponse represents the structure of the user details in the response.
type UserResponse struct {
	ID        uint      `json:"id"`
	Email     string    `json:"email"`
	Name      string    `json:"name"`
	CreatedAt time.Time `json:"created_at"`
}

// ErrorResponse represents the structure of an error response.
type ErrorResponse struct {
	Error string `json:"error"`
}

// ToAuthResponse converts an entity.User and JWT token into an AuthResponse.
func (p *authPresenterImpl) ToAuthResponse(user *entity.User, token string) AuthResponse {
	return AuthResponse{
		Token: token,
		User:  p.ToUserResponse(user),
	}
}

// ToUserResponse converts an entity.User into a UserResponse.
func (p *authPresenterImpl) ToUserResponse(user *entity.User) UserResponse {
	return UserResponse{
		ID:        user.ID,
		Email:     user.Email,
		Name:      user.Name,
		CreatedAt: user.CreatedAt,
	}
}

// ToErrorResponse converts an error message into an ErrorResponse.
func (p *authPresenterImpl) ToErrorResponse(err string) ErrorResponse {
	return ErrorResponse{
		Error: err,
	}
}
