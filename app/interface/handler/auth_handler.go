package handler

import (
	"encoding/json"
	"net/http"

	"smart_lock_back/interface/presenter"
	"smart_lock_back/usecase"
)

type AuthHandler struct {
	AuthUsecase *usecase.AuthUsecase
	Presenter   presenter.AuthPresenter
}

// NewAuthHandler creates a new instance of AuthHandler.
func NewAuthHandler(authUsecase *usecase.AuthUsecase, presenter presenter.AuthPresenter) *AuthHandler {
	return &AuthHandler{
		AuthUsecase: authUsecase,
		Presenter:   presenter,
	}
}

// Register handles user registration (POST /api/register).
func (h *AuthHandler) Register(w http.ResponseWriter, r *http.Request) {
	var req struct {
		Email    string `json:"email"`
		Password string `json:"password"`
		Name     string `json:"name"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid request body", http.StatusBadRequest)
		return
	}

	token, user, err := h.AuthUsecase.Register(req.Email, req.Password, req.Name)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	response := h.Presenter.ToAuthResponse(user, token)
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(response)
}

// Login handles user login (POST /api/login).
func (h *AuthHandler) Login(w http.ResponseWriter, r *http.Request) {
	var req struct {
		Email    string `json:"email"`
		Password string `json:"password"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid request body", http.StatusBadRequest)
		return
	}

	token, user, err := h.AuthUsecase.Login(req.Email, req.Password)
	if err != nil {
		http.Error(w, "Unauthorized", http.StatusUnauthorized)
		return
	}

	response := h.Presenter.ToAuthResponse(user, token)
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(response)
}

// Me handles fetching the authenticated user's details (GET /api/me).
func (h *AuthHandler) Me(w http.ResponseWriter, r *http.Request) {
	userID := r.Context().Value("user_id").(uint)

	user, err := h.AuthUsecase.GetUserByID(userID)
	if err != nil {
		http.Error(w, "User not found", http.StatusNotFound)
		return
	}

	response := h.Presenter.ToUserResponse(user)
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(response)
}
