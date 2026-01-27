package usecase

import (
	"errors"
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
