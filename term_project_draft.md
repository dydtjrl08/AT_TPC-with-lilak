# Term Project 초안 (기초계산과학)
## 주제: CNN을 이용한 PID 성능 개선

## 1) 프로젝트 배경
- 목표: detector 신호 기반 입자 식별(PID) 정확도 개선
- 대상 학생: 학부 2학년, 파이썬 초심자

## 2) 연구 질문
1. 기본 feature(dx, dy, dz, energy)만으로 baseline CNN 성능은 어느 정도인가?
2. 전처리(정규화/결측치 처리/특징 재표현)가 PID 성능에 미치는 영향은?
3. 왜 일부 feature 조합(예: dx/dy)은 잘 되고, dz/dy는 어려운가?

## 3) 필수 파이프라인
raw detector data -> preprocessing -> dataset -> loss -> optimizer/GD -> train/validation/test -> CNN -> evaluation -> visualization

## 4) 최소 구현 요구사항
- [ ] 데이터 분할: train/validation/test
- [ ] 모델: 1D CNN 또는 2D CNN (입력 표현 근거 제시)
- [ ] loss: CrossEntropyLoss
- [ ] optimizer: SGD 또는 Adam, 학습률 실험 2개 이상
- [ ] 평가: accuracy + confusion matrix + class별 precision/recall/F1
- [ ] 시각화: 학습 곡선 1개 이상, feature/오분류 시각화 1개 이상

## 5) 보고서 구성(권장)
1. 문제 정의와 물리적 배경
2. 데이터 설명 및 전처리 근거
3. 모델/학습 설정(loss, optimizer, epoch, batch size)
4. 결과(정량 지표 + 그림)
5. 실패 사례 분석(특히 dz 관련 성능 저하 원인)
6. 개선 아이디어(데이터 표현/모델/실험 설계)

## 6) 평가 루브릭 (100점)
- 문제 정의 명확성: 15점
- 파이프라인 완성도: 20점
- 실험 설계(비교군/하이퍼파라미터): 20점
- 해석 품질(왜 그런 결과가 나왔는지): 25점
- 재현성(코드/시드/환경/설명): 20점
