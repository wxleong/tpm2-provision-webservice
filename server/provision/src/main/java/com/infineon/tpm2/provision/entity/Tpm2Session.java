package com.infineon.tpm2.provision.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

@Entity
@Table(name = "tpm2_session")
@Getter
@Setter
public class Tpm2Session {
    public static final int SESSION_STAT_PENDING_CMD = 0;
    public static final int SESSION_STAT_PENDING_RESP = 1;
    public static final int SESSION_STAT_COMPLETED = 2;

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    private int rc;
    private String rc_message;
    private String command;
    private String response;
    private int status;
}
