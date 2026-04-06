# TPC AI Term Project 튜토리얼 플레이북 (nptool + LILAK + Geant4)

> 목적: 학생들이 **물리 의미(왜?)**와 **코드/브랜치(어디?)**를 같이 이해하면서,
> TPCDrum 데이터로 **고전 재구성(HT/Fitting method)** 과 **AI 회귀(CNN/트리 기반)** 를 비교 학습하도록 구성.

---

## 0) 현재 파이프라인 기준(고정)

### 데이터 체인
1. `STDDriftElectronMaker`
2. `STDElectronicsMaker`
3. `STDNoiseSubtractor`
4. `STDPulseAnalyser`

### 주요 브랜치
- 입력/MC: `MCTrack`, `MCVertex`, `MCStepTPCDrum`
- Digi: `RawPad`, `Hit`, `MCTag`, `MCTrack`, `MCStepTPCDrum`
- 추가 task 출력: `StepTruth`, `RecoSummary`, `HitTruth`

### 해석 기준(중요)
- **숫자 평가용 truth slope**: `MCTrack + vertex + GetCoordinateGeantToPad()`
- **display truth trajectory**: `MCStepTPCDrum`
- **Fitting method**: `Hit.x vs Hit.y` least-squares 직선
- **AI 예측**: event-level feature -> `true_dxdy`, `true_dzdy` 회귀

---

## 1) 튜토리얼 8세션 구성안 (물리 + 코드 + 브랜치 동시 설명)

## Session 1. PID + dE/dx 물리 + TPC 소개

- 물리 핵심:
  - Bethe-Bloch의 의미 (질량/전하/속도에 따른 평균 에너지 손실)
  - 저에너지/중간에너지/고에너지에서 우세한 상호작용 구분
  - Geant4에서 전자기 process가 트랙 형상 및 에너지 침적에 주는 영향
- 코드 대응:
  - `tpcdrum/geant4/TPCDrumConstruction.*` : 검출기/매질/기하
  - `tpcdrum/simulation/PhysicsListOption.txt` : 물리리스트 옵션
- 브랜치 대응:
  - `MCTrack`, `MCVertex`, `MCStepTPCDrum`로 입자 종류/진행/스텝 확인

## Session 2. Raw data와 Feature 개념

- 물리 핵심:
  - 전하 drift/증폭/전자회로 응답을 거치면서 원신호가 어떻게 관측량으로 바뀌는지
- 코드 대응:
  - `STDDriftElectronMaker`, `STDElectronicsMaker`, `STDNoiseSubtractor`, `STDPulseAnalyser`
- 브랜치 대응:
  - `RawPad` (파형), `Hit` (threshold 이후), `MCTag`

## Session 3. Pre-processing + HT/Fitting method + MC 비교

- 물리 핵심:
  - 선형 track 가정의 유효 구간과 실패 조건
- 코드 대응:
  - HT: `LKHTLineTracker` 기반 재구성(기존 SingleAlphas 방식)
  - Fitting method: 히트 포인트 최소제곱 직선
- 브랜치 대응:
  - `Hit` vs `MCTrack/MCStepTPCDrum` 동시 플롯

## Session 4. Dataset 정의

- 분류:
  - truth dataset (`.root` with MC truth)
  - 1차 reco dataset (`*_digi.root`, Hit 생성 완료)
  - AI 학습용 tabular dataset (`tpcd_ai_event.csv`, 필요시 hit/pad CSV)
- 핵심:
  - 레이블(`true_dxdy`, `true_dzdy`)과 입력 feature를 분리해 문서화

## Session 5. Loss function & 모델군 소개

- 물리/통계 핵심:
  - MAE, MSE, RMSE 해석
  - outlier 민감도와 실험 데이터 품질의 연결
- 모델군 비교:
  - RandomForest/XGBoost(설명력 높음)
  - MLP/CNN(성능 잠재력)
  - 지도/비지도 사용 시나리오

## Session 6. CNN + 하이퍼파라미터 실습

- 변경 축:
  - 채널 수, 커널 크기, depth, dropout, batch size, learning rate
- 결과 비교:
  - 성능(RMSE), 안정성(seed별 분산), 학습시간(실용성)

## Session 7. Train/Validation/Test 분할

- 실험 설계:
  - event-level split (누수 방지)
  - seed 고정/로그 저장
- 권장:
  - 70/15/15 또는 60/20/20

## Session 8. 최종 시각화 및 해석

- 같은 event에 대해 동시 표시:
  - pad color = `Hit.W`
  - hit center
  - truth trajectory (`MCStepTPCDrum`)
  - truth line (`MCTrack` vertex 기반)
  - HT reco line
  - Fitting method line
  - AI line
- 산출물:
  - residual overlay / 2D truth-vs-pred / per-event all-panels

---

## 2) 학생 프로젝트 주제 추천 (바로 시작 가능한 4개)

### 주제 A. "Fitting method를 이기는 AI 회귀기 만들기"
- 목표: `true_dxdy`, `true_dzdy` 예측에서 Fitting method 대비 MAE 20% 개선
- 입력: `tpcd_ai_event.csv`
- 베이스라인: RF + MLP
- 확장: CNN (2D pad image 또는 time-bucket map)

### 주제 B. "HT 실패 이벤트 자동 분류"
- 목표: HT residual이 큰 이벤트를 사전 탐지
- 라벨: `|HT - truth| > threshold`
- 활용: 온라인 quality flagging

### 주제 C. "PID 보조 feature 만들기"
- 목표: hit topology + dE/dx proxy로 입자군 분리 가능성 탐색
- 연결: Bethe-Bloch 개념을 데이터 feature로 연결

### 주제 D. "Domain gap 분석 (sim -> digi 설정 변화)"
- 목표: noise/threshold/drift parameter 변화 시 AI 강건성 비교
- 핵심: 물리 파라미터 변화에 대한 모델 민감도 정량화

---

## 3) 지금 바로 필요한 파일 체크리스트 (업로드/공유용)

1. Truth ROOT (`*.root`)  
2. Digi ROOT (`*_digi.root`, `RawPad/Hit/MCStepTPCDrum/MCTrack` 포함)  
3. AI CSV 생성 매크로 (`exportTPCDToCSV.C`)  
4. 비교 매크로 (`CompareHTWithFittingAndAI*.C`)  
5. baseline 노트북 (`tpcdrum_event_baseline_clean_colab.ipynb`)  
6. end-to-end 노트북/파이썬 (`tpcdrum_local_end_to_end.ipynb/.py`)  

> 위 6개가 있으면, 수업용 데이터 생성 -> 학습 -> 시각화까지 한 번에 재현 가능.

---

## 4) 튜토리얼 운영 팁 (실습 실패율 줄이기)

- 파일명 규칙 고정: `runXXX_true.root`, `runXXX_digi.root`, `runXXX_event.csv`
- seed/log 고정: 실습 보고서에서 재현성 확보
- 지표 3종 고정: MAE + RMSE + event display 정성평가
- 용어 고정: "CSV reco" 대신 **"Fitting method"** 사용

---

## 5) 즉시 실행 예시 커맨드

```bash
# 1) ROOT 구조 점검
root -l -q 'inspectSimFiles.C("data/run001_true.root","data/run001_digi.root")'

# 2) CSV 내보내기
root -l -q 'exportTPCDToCSV.C("data/run001_digi.root","data/run001_event.csv","data/run001_hit.csv","data/run001_pad.csv")'

# 3) 비교 플롯(Heavy 버전)
root -l -q 'CompareHTWithFittingAndAI_Heavy.C("data/run001_digi.root","data/run001_event.csv",42)'

# 4) 로컬 AI 베이스라인
python3 tpcdrum_local_end_to_end.py --event_csv data/run001_event.csv
```

---

## 6) 왜 이 구성이 교육적으로 좋은가?

- 물리식(Bethe-Bloch) -> 검출기 응답 -> 재구성 -> AI 추정으로 이어지는 **완전한 실험 파이프라인** 경험 가능
- "고전 방법의 한계"를 먼저 체감시킨 뒤 AI를 도입해서, 학생들이 AI를 "블랙박스"가 아니라 "필요한 도구"로 이해함
- 같은 이벤트를 여러 방법으로 겹쳐 그려서, 모델 성능의 물리적 의미(바이어스/분산/실패 패턴)를 직관적으로 설명 가능

