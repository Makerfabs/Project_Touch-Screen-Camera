#coding:utf-8
from flask import Flask, request
import os
app = Flask(__name__)
 
 
 
@app.route('/json', methods=['POST'])
def my_json(): 

    if request.method == 'POST':
        print ("request.headers: ", request.headers)
        #print(request.get_data(),hex)

        byte_file_name = request.headers['File-Name']

        if byte_file_name:
            file_path = os.path.join('./upload/', byte_file_name)
            print(file_path)
            rec_file = open(file_path,"wb")
            rec_file.write(request.get_data())
            rec_file.close()
            return 'Save successful'


    return 'From Server Response'
 
if __name__ == '__main__': 
    app.run("192.168.1.128", port=5002)