package handler

import (
	"encoding/json"
	"net/http"
	"strconv"

	"smart_lock_back/interface/presenter"
	"smart_lock_back/usecase"
)

type RoomHandler struct {
	RoomUsecase usecase.RoomUsecase
	Presenter   presenter.RoomPresenter
}

func NewRoomHandler(roomUsecase usecase.RoomUsecase, presenter presenter.RoomPresenter) *RoomHandler {
	return &RoomHandler{
		RoomUsecase: roomUsecase,
		Presenter:   presenter,
	}
}

// GetAllRooms handles fetching all rooms (GET /api/rooms)
func (h *RoomHandler) GetAllRooms(w http.ResponseWriter, r *http.Request) {
	rooms, err := h.RoomUsecase.GetAllRooms()
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Failed to fetch rooms"))
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(h.Presenter.PresentRooms(rooms))
}

// GetMyBookings handles fetching user's bookings (GET /api/my-bookings)
func (h *RoomHandler) GetMyBookings(w http.ResponseWriter, r *http.Request) {
	userID := r.Context().Value("user_id").(uint)

	bookings, err := h.RoomUsecase.GetUserRooms(userID)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Failed to fetch bookings"))
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(h.Presenter.PresentUserRooms(bookings))
}

// BookRoom handles room booking (POST /api/book-room)
func (h *RoomHandler) BookRoom(w http.ResponseWriter, r *http.Request) {
	userID := r.Context().Value("user_id").(uint)

	var req struct {
		RoomID uint `json:"room_id"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Invalid request body"))
		return
	}

	if req.RoomID == 0 {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Room ID is required"))
		return
	}

	err := h.RoomUsecase.BookRoom(userID, req.RoomID)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(h.Presenter.PresentError(err.Error()))
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(map[string]interface{}{
		"message": "Room booked successfully",
	})
}

// CancelBooking handles booking cancellation (DELETE /api/bookings/:id)
func (h *RoomHandler) CancelBooking(w http.ResponseWriter, r *http.Request) {
	userID := r.Context().Value("user_id").(uint)

	// URL から booking ID を取得 (例: /api/bookings/1)
	bookingIDStr := r.URL.Query().Get("id")
	if bookingIDStr == "" {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Booking ID is required"))
		return
	}

	bookingID, err := strconv.ParseUint(bookingIDStr, 10, 32)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Invalid booking ID"))
		return
	}

	err = h.RoomUsecase.CancelBooking(userID, uint(bookingID))
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Failed to cancel booking"))
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(map[string]interface{}{
		"message": "Booking cancelled successfully",
	})
}

// GetBleUuid handles fetching BLE UUID by room ID (GET /api/ble-uuid?room_id=1)
func (h *RoomHandler) GetBleUuid(w http.ResponseWriter, r *http.Request) {
	roomIDStr := r.URL.Query().Get("room_id")
	if roomIDStr == "" {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(map[string]interface{}{"error": "room_id is required"})
		return
	}

	roomID, err := strconv.ParseUint(roomIDStr, 10, 32)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(map[string]interface{}{"error": "invalid room_id"})
		return
	}

	uuids, err := h.RoomUsecase.GetBleUuidByRoomID(uint(roomID))
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		json.NewEncoder(w).Encode(map[string]interface{}{"error": "failed to fetch ble_uuid"})
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(map[string]interface{}{
		"ble_uuids": uuids,
	})
}

// GetRoomKey handles fetching the public key for a booked room (GET /api/rooms/:id/key)
func (h *RoomHandler) GetRoomKey(w http.ResponseWriter, r *http.Request) {
	userID := r.Context().Value("user_id").(uint)

	// URLからroom_idを取得
	roomIDStr := r.URL.Query().Get("room_id")
	if roomIDStr == "" {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Room ID is required"))
		return
	}

	roomID, err := strconv.ParseUint(roomIDStr, 10, 32)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		json.NewEncoder(w).Encode(h.Presenter.PresentError("Invalid room ID"))
		return
	}

	publicKey, err := h.RoomUsecase.GetRoomKeyForUser(userID, uint(roomID))
	if err != nil {
		w.WriteHeader(http.StatusForbidden)
		json.NewEncoder(w).Encode(h.Presenter.PresentError(err.Error()))
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(map[string]interface{}{
		"room_id":    roomID,
		"public_key": publicKey,
	})
}
