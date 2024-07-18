package com.infineon.tpm2.provision.repository;

import com.infineon.tpm2.provision.entity.Tpm2Session;
import org.springframework.data.jpa.repository.JpaRepository;

public interface Tpm2SessionRepository extends JpaRepository<Tpm2Session, Long> {

}
