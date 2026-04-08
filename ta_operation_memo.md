# 조교용 운영 메모 (초안)

## 1) 수업 운영 목표
- 학생이 파이프라인 전체를 "돌려보는 것"보다 "왜 필요한지 설명할 수 있는 것"에 초점.
- 반복 개념 설명 최소화: 핵심 개념은 처음에만 정리, 이후에는 적용/질문 중심.

## 2) 권장 진행안 (120분)
- 0~20분: 문제정의 + 파이프라인 개요
- 20~45분: raw -> preprocessing -> dataset 실습
- 45~75분: loss/optimizer/GD + train/val/test
- 75~100분: CNN 학습 및 평가
- 100~120분: 시각화 + dz/dy 한계 reasoning 토론

## 3) 자주 나오는 질문 대응
- Q: accuracy가 올라가면 다 좋은가요?
  - A: class 불균형/오분류 패턴을 같이 봐야 함(혼동행렬 필수).
- Q: 왜 dz가 특히 어렵죠?
  - A: drift 관련 불확실성, diffusion, detector response 변동이 누적되기 때문.

## 4) 채점 및 피드백 포인트
- 코드 실행 여부만 보지 말고, "근거 있는 해석"을 핵심으로 채점.
- 특히 실패 사례 분석을 강조(잘 안 된 이유를 쓰는 팀 가점).

## 5) 준비물 체크
- Python 환경(권장: 3.10+), numpy/pandas/scikit-learn/torch/matplotlib
- notebook 실행 확인
- 데이터 파일 경로 사전 점검
