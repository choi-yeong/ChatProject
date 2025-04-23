import socket
import threading
import tkinter as tk
from tkinter import messagebox, ttk
import json
import os

DB_PATH = "users.json"
IP_ADRS = "192.168.1.223"
PORT = 15410

def login_user(username, password):
    if not os.path.exists(DB_PATH): #유저목록이 없을경우 false 반환
        return False
    with open(DB_PATH, 'r') as f: #유저목록을 읽고 f라고 별칭 후 db에 저장
        db = json.load(f)
    for user in db.get("users", []): #db에서 유저목록을 user로 대입 후 for문 동작
        if user["name"] == username and user["password"] == password: #유저의 이름과 유저네임, 유저패스워드와 패스워드가 동일하면 True반환
            return True
    return False # 동작에 실패한 경우 False 반환

def register_user(username, password):
    if os.path.exists(DB_PATH): #유저 목록이 있으면 유저목록을 읽고 f라고 별칭후 db에 저장
        with open(DB_PATH, 'r') as f:
            db = json.load(f)
    else:
        db = {"users": []} #유저목록이 없을 경우 db에 유저목록 저장

    for user in db["users"]:#db에 있는 유저목록을 for문 동작
        if user["name"] == username:  #유저이름 유저네임이 겹칠경우 False 반환
            return False

    db["users"].append({"name": username, "password": password}) #유저목록에 입력한 유저네임,패스워드를 추가
    with open(DB_PATH, 'w') as f:#유저목록을 쓰기모드로 오픈 후 db를 덤프저장.
        json.dump(db, f, indent=4) 
    return True

class LoginFrame(tk.Frame):
    def __init__(self, master):
        super().__init__(master, bg='#f3e1ff')
        self.master = master
        tk.Label(self, text="아이디", font=("Arial", 12), bg='#f3e1ff').grid(row=0, column=0, pady=5, sticky='e')
        tk.Label(self, text="비밀번호", font=("Arial", 12), bg='#f3e1ff').grid(row=1, column=0, pady=5, sticky='e')
        self.entry_user = tk.Entry(self, font=("Arial", 12))
        self.entry_pass = tk.Entry(self, show="*", font=("Arial", 12))
        self.entry_user.grid(row=0, column=1, pady=5)
        self.entry_pass.grid(row=1, column=1, pady=5)
        tk.Button(self, text="로그인", command=self.login, font=("Arial", 12), bg="#f4e1ff").grid(row=2, column=0, pady=10)
        tk.Button(self, text="회원가입", command=self.register, font=("Arial", 12), bg="#f4e1ff").grid(row=2, column=1, pady=10)

    def login(self):
        user = self.entry_user.get().strip()
        pw = self.entry_pass.get().strip()

        if not user or not pw:
            messagebox.showwarning("입력 오류", "아이디와 비밀번호를 모두 입력해주세요.")
            return

        if login_user(user, pw):
            self.master.show_chat(user)
        else:
            messagebox.showerror("로그인 실패", "아이디나 비밀번호가 틀립니다.")

    def register(self):
        user = self.entry_user.get().strip()
        pw = self.entry_pass.get().strip()

        if not user or not pw:
            messagebox.showwarning("입력 오류", "아이디와 비밀번호를 모두 입력해주세요.")
            return

        if register_user(user, pw): #True를 반환한경우 성공메시지 팝업
            messagebox.showinfo("회원가입 성공", "회원가입이 완료되었습니다. 로그인해주세요.")
        else:
            messagebox.showerror("회원가입 실패", "이미 존재하는 아이디입니다.")

class ChatClient(tk.Frame):
    def __init__(self, master, username):
        super().__init__(master, bg='#f3e1ff')
        self.master = master
        self.username = username

        self.chat_frame = tk.Frame(self, bg='#f3e1ff')
        self.canvas = tk.Canvas(self.chat_frame, bg='#f3e1ff', highlightthickness=0)
        self.scrollbar = ttk.Scrollbar(self.chat_frame, orient="vertical", command=self.canvas.yview)
        self.scrollable_frame = tk.Frame(self.canvas, bg='#f3e1ff', width=400)

        self.scrollable_frame.bind(
            "<Configure>",
            lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all"))
        )

        self.canvas.create_window((0, 0), window=self.scrollable_frame,anchor="nw", width=400)
        self.canvas.configure(yscrollcommand=self.scrollbar.set)

        self.chat_frame.pack(fill="both", expand=True, padx=10, pady=10)
        self.canvas.pack(side="left", fill="both", expand=True)
        self.scrollbar.pack(side="right", fill="y")

        self.msg_entry = tk.Entry(self, width=40, font=("Arial", 12))
        self.msg_entry.pack(side=tk.LEFT, padx=(10, 0), pady=(0, 10))
        self.msg_entry.bind("<Return>", self.send_message)

        self.send_button = tk.Button(self, text="📨 보내기", command=self.send_message, font=("Arial", 12), bg="#f4e1ff")
        self.send_button.pack(side=tk.LEFT, padx=(5, 10), pady=(0, 10))

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.socket.connect((IP_ADRS, PORT))
            self.socket.send(self.username.encode('euc-kr'))  # 서버에 첫 메시지로 닉네임 전송
            threading.Thread(target=self.receive_messages, daemon=True).start()
        except Exception as e:
            self.add_message(f"[오류] 서버 연결 실패: {e}", sender="system")

    def send_message(self, event=None):
        msg = self.msg_entry.get().strip()
        if msg:
            try:
                full_msg = msg
                self.socket.send(full_msg.encode('euc-kr'))  # 여기에 전체 메시지를 보내야 함
                self.add_message(msg, sender=self.username)  # 여기서는 메시지만 보여줘도 됨
            except Exception as e:
                self.add_message(f"[오류] 메시지 전송 실패: {e}", sender="system")
            self.msg_entry.delete(0, tk.END)

    def receive_messages(self):
        while True:
            try:
                # 데이터를 읽어온다.
                data = self.socket.recv(1024)
                if not data:  # 연결이 끊어졌을 경우
                    break
                msg = data.decode('euc-kr')
                if msg.strip():
                # "닉네임: 메시지" 형식에서 닉네임과 메시지를 나눔
                    if ":" in msg:
                        nickname, text = msg.split(":", 1)
                        nickname = nickname.strip()
                        text = text.strip()

                        # 본인 메시지이면 오른쪽 정렬
                        if nickname == self.username:
                            self.add_message(text, sender=self.username)
                        else:
                            self.add_message(f"{nickname}: {text}", sender="other")
            except Exception as e:
                break  # 예외가 발생하면 종료
        self.add_message("서버 연결 종료됨.", sender="system")

    def add_message(self, text, sender):
        wrapper = tk.Frame(self.scrollable_frame, bg="#f3e1ff")
        wrapper.pack(fill="both", pady=3, anchor="e" if sender == self.username else "w", padx=5)

        if sender == self.username:
            bg_color = "#f9f1fe"
            anchor = "e"
            name = f"{self.username}(나)"
            wrapper.pack(fill="both", pady=3, anchor="e", padx=10)
        elif sender == "other":
            bg_color = "#f2f2f2"
            anchor = "w"
            if ": " in text:
                name, text = text.split(": ",1)
            else:
                name="익명의 상대방"
            wrapper.pack(fill="both", pady=3, anchor="w", padx=10)
        else:
            bg_color = "#ffecec"
            anchor = "center"
            name = "🔔"

        name_label = tk.Label(wrapper, text=name, font=("Arial", 9), bg='#f3e1ff', fg='#555')
        name_label.pack(anchor=anchor)

        msg_label = tk.Label(
            wrapper,
            text=text,
            font=("Arial", 12),
            bg=bg_color,
            wraplength=300,
            padx=10,
            pady=5,
            justify="left",
            relief="ridge",
            bd=2
        )
        msg_label.pack(anchor=anchor)

        self.canvas.update_idletasks()
        self.canvas.yview_moveto(1)

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("로그인창")
        self.geometry("450x550")
        self.configure(bg="#f3e1ff")
        self.current_frame = None
        self.show_login()

    def show_login(self):
        if self.current_frame:
            self.current_frame.destroy()
        self.title("로그인창")
        self.current_frame = LoginFrame(self)
        self.current_frame.pack(expand=True)

    def show_chat(self, username):
        if self.current_frame:
            self.current_frame.destroy()
        self.title(f"채팅창-{username}님")
        self.current_frame = ChatClient(self, username)
        self.current_frame.pack(expand=True, fill="both")

if __name__ == "__main__":
    app = App()
    app.mainloop()