package com.louisdouglas.AutoSecure.Location;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

@Service
public class LocationService {
    private final LocationRepository locationRepository;

    @Autowired
    public LocationService(LocationRepository locationRepository) {
        this.locationRepository = locationRepository;
    }

    public List<Location> getAllLocations() {
//        MqttService test = new MqttService();
//        test.publish();
//        MqttConfig test = new MqttConfig();
//        test.publishLocation("Hello there from get all locations!");
//        MqttSend send = new MqttSend();
        return locationRepository.findAll();
    }

    public Location getClientLocation(String user) {
        List<Location> location = locationRepository.findLocationByUser(user);
        if (location.isEmpty()) {
            return null;
        }
        return location.get(0);
    }

    public Location createLocation(Location location) {
        List<Location> oldLocation = locationRepository.findLocationByUser(location.getUser());
        // Replace if record already exists
        if (!oldLocation.isEmpty()) {
            locationRepository.delete(oldLocation.get(0));
        }
        return locationRepository.save(location);
    }

    // Other CRUD operations...
}