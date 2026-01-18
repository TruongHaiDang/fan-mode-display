# fan-mode-display

Present a small UI to notify the current fan mode

# Cách chạy dự án này dạng service trên ubuntu

## Bước 1: build dự án ra file thực thi sau đó đặt ở `/usr/local/bin`

```bash
sudo cp <exec_file> /usr/local/bin/
```

## Bước 2: tạo file service ở `~/.config/systemd/user/fan-mode-display.service`

```bash
[Unit]
Description=Fan Mode Display UI

[Service]
Type=simple
ExecStart=/usr/local/bin/fan-mode-display
Restart=on-failure
RestartSec=3

[Install]
WantedBy=graphical-session.target
```

## Bước 3: Kích hoạt service

```bash
systemctl --user daemon-reload
systemctl --user enable --now fan-mode-display.service
```

# Các lệnh khác

**Xem trạng thái status của service**

```bash
systemctl --user status fan-mode-display.service
```

**Khởi động/Dừng service**

```bash
systemctl --user start/stop fan-mode-display.service
```
