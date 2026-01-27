package database

import (
	"fmt"
	"log"
	"os"

	"gorm.io/driver/postgres"
	"gorm.io/gorm"
	"gorm.io/gorm/logger"
)

// グローバルDB変数
var DB *gorm.DB

// Initialize - DB接続を初期化する
func Initialize() error {
	// 環境変数からDB接続情報を取得
	host := os.Getenv("DB_HOST")
	user := os.Getenv("DB_USER")
	password := os.Getenv("DB_PASSWORD")
	dbname := os.Getenv("DB_NAME")
	port := os.Getenv("DB_PORT")

	// PostgreSQL接続文字列(DSN)を作成
	dsn := fmt.Sprintf(
		"host=%s user=%s password=%s dbname=%s port=%s sslmode=disable TimeZone=Asia/Tokyo",
		host, user, password, dbname, port,
	)

	// gorm.Open()でDB接続
	var err error
	DB, err = gorm.Open(postgres.Open(dsn), &gorm.Config{
		Logger: logger.Default.LogMode(logger.Info),
	})
	if err != nil {
		return fmt.Errorf("データベース接続に失敗しました: %w", err)
	}

	log.Println("データベース接続に成功しました")
	return nil
}

// GetDB - グローバルDB変数を返す
func GetDB() *gorm.DB {
	return DB
}
