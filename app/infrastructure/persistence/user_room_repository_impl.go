package persistence

import (
	"smart_lock_back/domain/entity"
	"smart_lock_back/domain/repository"

	"gorm.io/gorm"
)

type userRoomRepository struct {
	db *gorm.DB
}

func NewUserRoomRepository(db *gorm.DB) repository.UserRoomRepository {
	return &userRoomRepository{db: db}
}

func (r *userRoomRepository) Create(userRoom *entity.UserRoom) error {
	return r.db.Create(userRoom).Error
}

func (r *userRoomRepository) FindByUserID(userID uint) ([]*entity.UserRoom, error) {
	var userRooms []*entity.UserRoom
	if err := r.db.Preload("Room").Where("user_id = ?", userID).Find(&userRooms).Error; err != nil {
		return nil, err
	}
	return userRooms, nil
}

func (r *userRoomRepository) FindByRoomID(roomID uint) ([]*entity.UserRoom, error) {
	var userRooms []*entity.UserRoom
	if err := r.db.Preload("User").Where("room_id = ?", roomID).Find(&userRooms).Error; err != nil {
		return nil, err
	}
	return userRooms, nil
}

func (r *userRoomRepository) Delete(id uint) error {
	return r.db.Delete(&entity.UserRoom{}, id).Error
}

func (r *userRoomRepository) ExistsByUserAndRoom(userID uint, roomID uint) (bool, error) {
	var count int64
	err := r.db.Model(&entity.UserRoom{}).Where("user_id = ? AND room_id = ?", userID, roomID).Count(&count).Error
	return count > 0, err
}
