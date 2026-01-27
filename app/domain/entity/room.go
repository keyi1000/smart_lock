package entity

import "gorm.io/gorm"

type Room struct {
	gorm.Model
	RoomName string `gorm:"type:varchar(100);not null" json:"room_name"`
}
