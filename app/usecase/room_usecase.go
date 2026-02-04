package usecase

import (
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"smart_lock_back/domain/entity"
	"smart_lock_back/domain/repository"

	"github.com/google/uuid"
)

type RoomUsecase interface {
	BookRoom(userID uint, roomID uint) error
	GetUserRooms(userID uint) ([]*entity.UserRoom, error)
	GetAllRooms() ([]*entity.Room, error)
	CancelBooking(userID uint, bookingID uint) error
	GetBleUuidByRoomID(roomID uint) ([]string, error)
	GetRoomKeyForUser(userID uint, roomID uint) (string, error)
}

type roomUsecase struct {
	roomRepo     repository.RoomRepository
	userRoomRepo repository.UserRoomRepository
}

func NewRoomUsecase(roomRepo repository.RoomRepository, userRoomRepo repository.UserRoomRepository) RoomUsecase {
	return &roomUsecase{
		roomRepo:     roomRepo,
		userRoomRepo: userRoomRepo,
	}
}

func (u *roomUsecase) BookRoom(userID uint, roomID uint) error {
	// 部屋が存在するか確認
	_, err := u.roomRepo.FindByID(roomID)
	if err != nil {
		return errors.New("room not found")
	}

	// すでに予約されているか確認
	exists, err := u.userRoomRepo.ExistsByUserAndRoom(userID, roomID)
	if err != nil {
		return err
	}
	if exists {
		return errors.New("room already booked by this user")
	}

	// UUIDを生成
	bleUUID := uuid.New().String()

	// 予約を作成
	userRoom := &entity.UserRoom{
		UserID:  userID,
		RoomID:  roomID,
		BleUUID: bleUUID,
	}
	return u.userRoomRepo.Create(userRoom)
}

func (u *roomUsecase) GetUserRooms(userID uint) ([]*entity.UserRoom, error) {
	return u.userRoomRepo.FindByUserID(userID)
}

func (u *roomUsecase) GetAllRooms() ([]*entity.Room, error) {
	return u.roomRepo.FindAll()
}

func (u *roomUsecase) CancelBooking(userID uint, bookingID uint) error {
	// TODO: ユーザーが自分の予約のみキャンセルできるように検証を追加
	return u.userRoomRepo.Delete(bookingID)
}

func (u *roomUsecase) GetBleUuidByRoomID(roomID uint) ([]string, error) {
	userRooms, err := u.userRoomRepo.FindByRoomID(roomID)
	if err != nil {
		return nil, err
	}

	uuids := make([]string, 0, len(userRooms))
	for _, ur := range userRooms {
		if ur.BleUUID != "" {
			uuids = append(uuids, ur.BleUUID)
		}
	}

	return uuids, nil
}

func (u *roomUsecase) GetRoomKeyForUser(userID uint, roomID uint) (string, error) {
	// 部屋が存在するか確認
	room, err := u.roomRepo.FindByID(roomID)
	if err != nil {
		return "", errors.New("room not found")
	}

	// ユーザーがこの部屋を予約しているか確認
	hasBooking, err := u.userRoomRepo.ExistsByUserAndRoom(userID, roomID)
	if err != nil {
		return "", err
	}
	if !hasBooking {
		return "", errors.New("you have not booked this room")
	}

	// 8081番ポートの鍵サーバーにリクエスト
	keyServerURL := fmt.Sprintf("http://localhost:8081/api/keys/%s/public", room.RoomName)
	resp, err := http.Get(keyServerURL)
	if err != nil {
		return "", fmt.Errorf("failed to fetch key from key server: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("key server returned status: %d", resp.StatusCode)
	}

	var keyResponse struct {
		PublicKey string `json:"public_key"`
	}
	if err := json.NewDecoder(resp.Body).Decode(&keyResponse); err != nil {
		return "", fmt.Errorf("failed to decode key response: %v", err)
	}

	return keyResponse.PublicKey, nil
}
