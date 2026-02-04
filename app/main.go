package main

import (
	"log"
	"net/http"

	"smart_lock_back/domain/entity"
	"smart_lock_back/infrastructure/persistence"
	"smart_lock_back/interface/handler"
	"smart_lock_back/interface/middleware"
	"smart_lock_back/interface/presenter"
	"smart_lock_back/usecase"

	"smart_lock_back/infrastructure/database"

	"gorm.io/gorm"
)

func main() {
	// Initialize the database connection
	if err := database.Initialize(); err != nil {
		log.Fatalf("Failed to initialize database: %v", err)
	}

	db := database.GetDB()

	// Auto migrate the database tables
	log.Println("Starting database migration...")
	if err := db.AutoMigrate(&entity.User{}, &entity.Room{}, &entity.UserRoom{}); err != nil {
		log.Fatalf("Failed to migrate database: %v", err)
	}
	log.Println("Database migration completed successfully")

	// 初期データを作成
	var roomCount int64
	db.Model(&entity.Room{}).Count(&roomCount)
	if roomCount == 0 {
		log.Println("Creating initial room data...")
		initialRooms := []entity.Room{
			{Model: gorm.Model{ID: 101}, RoomName: "101"},
			{Model: gorm.Model{ID: 102}, RoomName: "102"},
			{Model: gorm.Model{ID: 103}, RoomName: "103"},
		}
		for _, room := range initialRooms {
			if err := db.Create(&room).Error; err != nil {
				log.Printf("Failed to create room %s: %v", room.RoomName, err)
			}
		}
		log.Println("Initial room data created successfully")
	}

	// Initialize repositories
	userRepo := persistence.NewUserRepository(db)
	roomRepo := persistence.NewRoomRepository(db)
	userRoomRepo := persistence.NewUserRoomRepository(db)

	// Initialize use cases
	authUsecase := usecase.NewAuthUsecase(userRepo)
	roomUsecase := usecase.NewRoomUsecase(roomRepo, userRoomRepo)

	// Initialize presenters
	authPresenter := presenter.NewAuthPresenter()
	roomPresenter := presenter.NewRoomPresenter()

	// Initialize handlers
	authHandler := handler.NewAuthHandler(authUsecase, authPresenter)
	roomHandler := handler.NewRoomHandler(roomUsecase, roomPresenter)

	// Define routes
	http.HandleFunc("/api/register", authHandler.Register)
	http.HandleFunc("/api/login", authHandler.Login)
	http.HandleFunc("/api/me", middleware.AuthMiddleware(authHandler.Me))

	// Room routes
	http.HandleFunc("/api/rooms", roomHandler.GetAllRooms)
	http.HandleFunc("/api/my-bookings", middleware.AuthMiddleware(roomHandler.GetMyBookings))
	http.HandleFunc("/api/book-room", middleware.AuthMiddleware(roomHandler.BookRoom))
	http.HandleFunc("/api/cancel-booking", middleware.AuthMiddleware(roomHandler.CancelBooking))
	http.HandleFunc("/api/rooms/keys", middleware.AuthMiddleware(roomHandler.GetRoomKey))
	// Smart lock route (no authentication)
	http.HandleFunc("/api/ble-uuid", roomHandler.GetBleUuid)

	// Health check endpoint
	http.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("nice"))
	})

	// Start the server
	log.Println("Server is running on :8080")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatalf("Failed to start server: %v", err)
	}
}
