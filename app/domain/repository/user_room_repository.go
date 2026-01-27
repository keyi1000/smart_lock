package repository

import "smart_lock_back/domain/entity"

type UserRoomRepository interface {
	Create(userRoom *entity.UserRoom) error
	FindByUserID(userID uint) ([]*entity.UserRoom, error)
	FindByRoomID(roomID uint) ([]*entity.UserRoom, error)
	Delete(id uint) error
	ExistsByUserAndRoom(userID uint, roomID uint) (bool, error)
}
