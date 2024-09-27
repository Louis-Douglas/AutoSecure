import 'dart:async';
import 'dart:convert';

import 'package:english_words/english_words.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:google_maps_flutter/google_maps_flutter.dart';
import 'package:http/http.dart' as http;
import 'Location.dart';
import 'dart:io';

String clientId = "ClientID";

Future<Location> fetchLocation() async {

  final response = await http
      .get(Uri.parse('ServerIPAndPath/$clientId'));
  if (response.statusCode == 200) {
    // If the server did return a 200 OK response,
    // then parse the JSON.
    debugPrint(response.body);
    return Location.fromJson(jsonDecode(response.body) as Map<String, dynamic>);
    // return test;
  } else {
    // If the server did not return a 200 OK response,
    // then throw an exception.
    throw Exception('Failed to load location');
  }
}


class MyHttpOverrides extends HttpOverrides{
  @override
  HttpClient createHttpClient(SecurityContext? context){
    return super.createHttpClient(context)
      ..badCertificateCallback = (X509Certificate cert, String host, int port)=> true;
  }
}

void main() {
  HttpOverrides.global = MyHttpOverrides();
  runApp(MyApp());
}



class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {

  late Future<Location> futureLocation;
  late GoogleMapController mapController;

  final LatLng _center = const LatLng(-33.86, 151.20);

  void _onMapCreated(GoogleMapController controller) {
    mapController = controller;
  }




  @override
  void initState() {
    super.initState();
    futureLocation = fetchLocation();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Fetch Data Example',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
      ),
      home: Scaffold(
        appBar: AppBar(
          title: const Text('AutoSecure'),
        ),
        body: Center(
          child: FutureBuilder<Location>(
            future: futureLocation,
            builder: (context, snapshot) {
              if (snapshot.hasData) {
                var data = snapshot.data!;
                return buildGoogleMap(data);
                // return Text("Latitude: ${data.latitude} - Longitude: ${data.longitude}");
              } else if (snapshot.hasError) {
                return Text('${snapshot.error}');
              }

              // By default, show a loading spinner.
              return const CircularProgressIndicator();
            },
          ),
        ),
      ),
    );
  }

  GoogleMap buildGoogleMap(Location data) {
    LatLng location = LatLng(double.parse(data.latitude), double.parse(data.longitude));
    return GoogleMap(
                onMapCreated: _onMapCreated,
                initialCameraPosition: CameraPosition(
                  target: location,
                  zoom: 11.0,
                ),
                markers: {
                  Marker(
                    markerId: MarkerId('Vehicle'),
                    position: location,
                    infoWindow: InfoWindow(
                      title: "Mini",
                      snippet: "Last update: ${data.updateDate}",
                    ),
                  )
                },
              );
  }
}