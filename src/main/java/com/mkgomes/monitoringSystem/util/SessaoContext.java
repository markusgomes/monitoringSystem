package com.mkgomes.monitoringSystem.util;

import org.springframework.stereotype.Component;

@Component
public class SessaoContext {
    private volatile Long sessaoAtualId;

    public Long getSessaoAtualId() {
        return sessaoAtualId;
    }

    public void setSessaoAtualId(Long sessaoAtualId) {
        this.sessaoAtualId = sessaoAtualId;
    }

}
