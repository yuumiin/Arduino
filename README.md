# Arduino

## sensor control
<!-- ### 1. reedswitch -->
- `reedswitch` 사용하여 모터 제어
  - [code_reedswitch](https://github.com/yuumiin/Arduino/blob/master/arduino_sensor/reedswitch/reedswitch.ino)
  - [code](https://github.com/yuumiin/Arduino/blob/master/0302_3/0302_3.ino) 
- `ad8465` 사용해서 온도 데이터 read
  - [code](https://github.com/yuumiin/Arduino/blob/master/ad8495/ad8495.ino)
- `hx711` 사용해서 로드셀 데이터 read
  - [code](https://github.com/yuumiin/Arduino/blob/master/arduino_sensor/hx711/hx711.ino)
- `lvdt` 사용해서 거리측정
  - [code](https://github.com/yuumiin/Arduino/blob/master/arduino_sensor/lvdt/lvdt.ino)
- `steppermotor` 제어
  - [code](https://github.com/yuumiin/Arduino/blob/master/arduino_sensor/steppermotor/steppermotor.ino)
  -  


## PID control
- version 1
  - [code](https://github.com/yuumiin/Arduino/blob/master/pid/pid.ino)
- version 2
  - [code](https://github.com/yuumiin/Arduino/blob/master/PID_temperature/PID_temperature.ino)
- 최종 PID control
  - [code](https://github.com/yuumiin/Arduino/blob/master/LPF/LPF.ino)
 
## SMU

## study
### 1. pyqt 연동
- arduino와 pyqt를 연결하여 서보모터 제어
  - [code](https://github.com/yuumiin/Arduino/blob/master/arduino_pyqt_connect/sketch_jan05a/sketch_jan05a.ino)




### 1. Contact Angle
- 실리콘의 친수성을 확인하기 위해 표면과 물방울사이 접촉각을 측정
- code
  - [실시간 접촉각 측정](https://github.com/yuumiin/ComputerVision/blob/main/contactangle.py)
  - [영상 저장](https://github.com/yuumiin/ComputerVision/blob/main/contact_angle_save.py)

### 2. 히스토그램 이미지 분류
- AI project 에서 사용될 CNN 분류에서 사용될 이미지 전처리
- code
  - [histogram code](https://github.com/yuumiin/ComputerVision/blob/main/preprocessing_hist.py)  

### 2. 히스토그램 이미지 분류
- YOLO 에 사용될 이미지 데이터 추출과 labeling
- code
  - [yolo 이미지 전처리 code](https://github.com/yuumiin/ComputerVision/blob/main/detect_contour.py)
