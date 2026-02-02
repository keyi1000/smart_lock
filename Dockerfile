# ビルドステージ
FROM golang:1.21-alpine AS builder

WORKDIR /app

# 依存関係のキャッシュ
COPY app/go.mod app/go.sum ./
RUN go mod download

# ソースコードをコピー
COPY app/ .

# バイナリをビルド
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o main .

# 実行ステージ
FROM alpine:latest

RUN apk --no-cache add ca-certificates tzdata

WORKDIR /root/

# ビルドステージからバイナリをコピー
COPY --from=builder /app/main .

# タイムゾーンを設定
ENV TZ=Asia/Tokyo

EXPOSE 8080

CMD ["./main"]
