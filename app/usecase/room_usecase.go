package usecase

import (
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"os"
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
	GetRoomKeysForUser(userID uint) ([]map[string]interface{}, error)
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

func (u *roomUsecase) GetRoomKeysForUser(userID uint) ([]map[string]interface{}, error) {
	// ユーザーの予約を取得
	userRooms, err := u.userRoomRepo.FindByUserID(userID)
	if err != nil {
		return nil, err
	}

	if len(userRooms) == 0 {
		return nil, errors.New("you have no room bookings")
	}

	results := make([]map[string]interface{}, 0, len(userRooms))

	// 環境変数から鍵サーバーのURLを取得
	keyServerBaseURL := os.Getenv("KEY_SERVER_URL")
	if keyServerBaseURL == "" {
		keyServerBaseURL = "http://192.168.11.24:8081" // デフォルト値
	}

	for _, userRoom := range userRooms {
		// 部屋情報を取得
		room, err := u.roomRepo.FindByID(userRoom.RoomID)
		if err != nil {
			continue
		}

		// 鍵サーバーにリクエスト
		keyServerURL := fmt.Sprintf("%s/api/keys/%s/public", keyServerBaseURL, room.RoomName)
		resp, err := http.Get(keyServerURL)
		if err != nil {
			continue
		}

		if resp.StatusCode == http.StatusOK {
			var keyResponse struct {
				PublicKey string `json:"public_key"`
			}
			if err := json.NewDecoder(resp.Body).Decode(&keyResponse); err == nil {
				results = append(results, map[string]interface{}{
					"room_id":    room.ID,
					"room_name":  room.RoomName,
					"public_key": keyResponse.PublicKey,
					"booking_id": userRoom.ID,
				})
			}
		}
		resp.Body.Close()
	}

	if len(results) == 0 {
		return nil, errors.New("failed to fetch keys from key server")
	}

	return results, nil
}
