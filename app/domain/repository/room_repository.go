package repository

import "smart_lock_back/domain/entity"

type RoomRepository interface {
	FindByID(id uint) (*entity.Room, error)
	FindAll() ([]*entity.Room, error)
}
