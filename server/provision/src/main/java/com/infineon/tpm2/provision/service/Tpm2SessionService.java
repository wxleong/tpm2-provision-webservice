package com.infineon.tpm2.provision.service;

import com.infineon.tpm2.provision.entity.Tpm2Session;
import com.infineon.tpm2.provision.repository.Tpm2SessionRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class Tpm2SessionService {
    @Autowired
    private Tpm2SessionRepository tpm2SessionRepository;

    public Tpm2Session findById(long id) {
        return tpm2SessionRepository.findById(id).orElse(null);
    }
}
