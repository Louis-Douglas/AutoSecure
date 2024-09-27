package com.louisdouglas.AutoSecure.Location;

import com.fasterxml.jackson.annotation.JsonProperty;
import jakarta.persistence.*;

@Entity
@Table(name = "locations")
public class Location {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;
    @JsonProperty
    @Column(name = "user")
    private String user;

    @JsonProperty
    @Column(name = "latitude")
    private String latitude;

    @JsonProperty
    @Column(name = "longitude")
    private String longitude;

    @JsonProperty
    @Column(name = "update_date")
    private String update_date;


    public String getUser() {
        return user;
    }
}