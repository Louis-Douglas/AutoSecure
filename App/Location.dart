class Location {
  final String user;
  final String latitude;
  final String longitude;
  final String updateDate;

  const Location({
    required this.user,
    required this.latitude,
    required this.longitude,
    required this.updateDate,
  });

  factory Location.fromJson(Map<String, dynamic> json) {
    return switch (json) {
      {
      'user': String user,
      'latitude': String latitude,
      'longitude': String longitude,
      'update_date': String updateDate,
      } =>
          Location(
            user: user,
            latitude: latitude,
            longitude: longitude,
            updateDate: updateDate,
          ),
      _ => throw const FormatException('Failed to load location.'),
    };
  }
}