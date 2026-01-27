package presenter

import (
	"smart_lock_back/domain/entity"
	"time"
)

type RoomPresenter interface {
	PresentRoom(room *entity.Room) map[string]interface{}
	PresentRooms(rooms []*entity.Room) map[string]interface{}
	PresentUserRoom(userRoom *entity.UserRoom) map[string]interface{}
	PresentUserRooms(userRooms []*entity.UserRoom) map[string]interface{}
	PresentError(message string) map[string]interface{}
}

type roomPresenter struct{}

func NewRoomPresenter() RoomPresenter {
	return &roomPresenter{}
}

func (p *roomPresenter) PresentRoom(room *entity.Room) map[string]interface{} {
	return map[string]interface{}{
		"id":         room.ID,
		"room_name":  room.RoomName,
		"created_at": room.CreatedAt.Format(time.RFC3339),
	}
}

func (p *roomPresenter) PresentRooms(rooms []*entity.Room) map[string]interface{} {
	roomList := make([]map[string]interface{}, len(rooms))
	for i, room := range rooms {
		roomList[i] = p.PresentRoom(room)
	}
	return map[string]interface{}{
		"rooms": roomList,
		"count": len(rooms),
	}
}

func (p *roomPresenter) PresentUserRoom(userRoom *entity.UserRoom) map[string]interface{} {
	result := map[string]interface{}{
		"id":         userRoom.ID,
		"user_id":    userRoom.UserID,
		"room_id":    userRoom.RoomID,
		"ble_uuid":   userRoom.BleUUID,
		"created_at": userRoom.CreatedAt.Format(time.RFC3339),
	}

	if userRoom.Room.ID != 0 {
		result["room"] = p.PresentRoom(&userRoom.Room)
	}

	return result
}

func (p *roomPresenter) PresentUserRooms(userRooms []*entity.UserRoom) map[string]interface{} {
	bookingList := make([]map[string]interface{}, len(userRooms))
	for i, userRoom := range userRooms {
		bookingList[i] = p.PresentUserRoom(userRoom)
	}
	return map[string]interface{}{
		"bookings": bookingList,
		"count":    len(userRooms),
	}
}

func (p *roomPresenter) PresentError(message string) map[string]interface{} {
	return map[string]interface{}{
		"error": message,
	}
}
