void setupTime() {
  timeClient.begin();
  timeClient.update();
}

String getHoursStr() {
  unsigned long hours = timeClient.getHours();
  return hours < 10 ? "0" + String(hours) : String(hours);
}

String getMinutesStr() {
  unsigned long minutes = timeClient.getMinutes();
  return minutes < 10 ? "0" + String(minutes) : String(minutes);
}
