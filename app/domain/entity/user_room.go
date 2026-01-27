package entity

import "gorm.io/gorm"

type UserRoom struct {
	gorm.Model
	UserID  uint   `gorm:"not null;index" json:"user_id"`
	RoomID  uint   `gorm:"not null;index" json:"room_id"`
	BleUUID string `gorm:"type:varchar(255)" json:"ble_uuid"`
	User    User   `gorm:"foreignKey:UserID" json:"user,omitempty"`
	Room    Room   `gorm:"foreignKey:RoomID" json:"room,omitempty"`
}
