FROM python:3.9-slim

WORKDIR /app

COPY subscriber.py .

RUN pip install -r requirements.txt

CMD ["python", "subscriber.py"]