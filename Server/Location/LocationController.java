package com.louisdouglas.AutoSecure.Location;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@RestController
@RequestMapping("/api/locations")
public class LocationController {
    private final LocationService locationService;

    @Autowired
    public LocationController(LocationService locationService) {
        this.locationService = locationService;
    }

    @GetMapping
    public List<Location> getAllLocations() {
        return locationService.getAllLocations();
    }

    @GetMapping("/{user}")
    public ResponseEntity<Location> getClientsLocation(@PathVariable String user) {
        Location location = locationService.getClientLocation(user);
        return location != null ? ResponseEntity.ok(location) : ResponseEntity.notFound().build();
    }

    @PostMapping
    public ResponseEntity<Location> createLocation(@RequestBody Location location) {
        Location savedLocation = locationService.createLocation(location);
        return ResponseEntity.status(HttpStatus.CREATED).body(savedLocation);
    }

    // Other CRUD endpoints...
}