import asyncio
import websockets
import socket

TCP_HOST = '127.0.0.1'  # C++ 서버 주소
TCP_PORT = 15410        # C++ 서버 포트

async def handle_websocket(websocket, path):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as tcp_sock:
        try:
            tcp_sock.connect((TCP_HOST, TCP_PORT))
        except Exception as e:
            await websocket.send("[에러] TCP 서버 연결 실패")
            return

        while True:
            try:
                message = await websocket.recv()
                tcp_sock.sendall(message.encode())

                data = tcp_sock.recv(1024)
                if not data:
                    break
                await websocket.send(data.decode())
            except Exception as e:
                await websocket.send("[에러] 연결 종료됨")
                break

start_server = websockets.serve(handle_websocket, "localhost", 8080)
print("WebSocket 브릿지 서버가 8080에서 실행 중...")
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
