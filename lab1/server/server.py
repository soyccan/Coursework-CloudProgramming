from flask import Flask

app = Flask(__name__)

@app.route('/')
def get():
    with open('data/msg.txt', 'w') as f:
        f.write("Hi!\n")
    return "Hi!"

app.run()
