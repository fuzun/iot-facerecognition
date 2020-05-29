from setuptools import setup

setup(
    name='iot-facerecognition-client',
    version='1.1.1',
    py_modules=['iot_facerecognition_client'],
    install_requires=['opencv-python', 'websocket-client', 'screeninfo'],
    entry_points={
        'console_scripts': [
            'iot-facerecognition-client = iot_facerecognition_client:main'
        ]},
    url='https://github.com/fuzun/iot-facerecognition',
    license='GPLv3',
    author='fuzun',
    author_email='fuzun54@outlook.com',
    description='python client for iot-facerecognition-server'
)
