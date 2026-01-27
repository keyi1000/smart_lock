package persistence

import (
	"smart_lock_back/domain/entity"
	"smart_lock_back/domain/repository"

	"gorm.io/gorm"
)

type roomRepository struct {
	db *gorm.DB
}

func NewRoomRepository(db *gorm.DB) repository.RoomRepository {
	return &roomRepository{db: db}
}

func (r *roomRepository) FindByID(id uint) (*entity.Room, error) {
	var room entity.Room
	if err := r.db.First(&room, id).Error; err != nil {
		return nil, err
	}
	return &room, nil
}

func (r *roomRepository) FindAll() ([]*entity.Room, error) {
	var rooms []*entity.Room
	if err := r.db.Find(&rooms).Error; err != nil {
		return nil, err
	}
	return rooms, nil
}
