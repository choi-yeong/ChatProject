from flask import Flask, render_template

app = Flask(__name__)

@app.route("/")
def home():
    return render_template("index.html")  # templates 폴더의 index.html 반환

if __name__ == "__main__":
    app.run(debug=True)
