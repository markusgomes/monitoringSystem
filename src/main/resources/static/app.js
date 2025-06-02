document.addEventListener('DOMContentLoaded', function () {
    // 1) Declarações de variáveis e elementos
    const cycleForm     = document.getElementById('cycleForm');
    const btnDownload   = document.getElementById('btnDownload');
    const timerContainer= document.getElementById('timerContainer');
    const timerDisplay  = document.getElementById('timerDisplay');

    let timerIntervalId = null;
    let segsDecorridos  = 0;
    let duracaoTotalSeg = 0;

    // 2) Declaração de formatarTempo
    function formatarTempo(segs) {
        const minutos  = Math.floor(segs / 60);
        const segundos = segs % 60;
        const mStr     = minutos.toString().padStart(2, '0');
        const sStr     = segundos.toString().padStart(2, '0');
        return `${mStr}:${sStr}`;
    }

    // 3) Declaração de iniciarCronometro (que usa formatarTempo)
    function iniciarCronometro(duracao) {
        segsDecorridos = 0;
        duracaoTotalSeg = duracao * 60;
        timerDisplay.textContent = '00:00';
        timerContainer.style.display = 'block';

        if (timerIntervalId !== null) {
            clearInterval(timerIntervalId);
        }

        // Aqui ocorrerá o erro se formatarTempo não estiver definido antes:
        timerIntervalId = setInterval(() => {
            segsDecorridos++;
            // ← formata o tempo (MM:SS)
            timerDisplay.textContent = formatarTempo(segsDecorridos);
            if (segsDecorridos >= duracaoTotalSeg) {
                clearInterval(timerIntervalId);
                timerIntervalId = null;
            }
        }, 1000);
    }

    // 4) Declaração de pararCronometro, submit do formulário, etc.
    function pararCronometro() {
        if (timerIntervalId !== null) {
            clearInterval(timerIntervalId);
            timerIntervalId = null;
        }
    }

    cycleForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        const sensores = Array.from(
            document.querySelectorAll('input[name="sensors"]:checked')
        ).map(el => el.value);
        const duracao = parseInt(document.getElementById('duracao').value, 10);

        if (sensores.length === 0) {
            alert('Selecione pelo menos um sensor');
            return;
        }

        btnDownload.disabled = true;
        btnDownload.style.opacity = 0.5;

        // → aqui chamamos iniciarCronometro, que chama formatarTempo
        iniciarCronometro(duracao);

        const response = await fetch('/iniciar', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify({
                usuario: 1,
                sensores,
                duracao
            })
        });

        const data = await response.json();
        if (response.ok) {
            alert(data.mensagem);
            window.sessaoId = data.id;
            const delayMs = duracao * 60 * 1000;
            setTimeout(() => {
                pararCronometro();
                btnDownload.disabled = false;
                btnDownload.style.opacity = 1;
            }, delayMs);
        } else {
            alert(data.erro || 'Erro ao iniciar sessão');
            pararCronometro();
            timerContainer.style.display = 'none';
        }
    });

    window.downloadCSV = function () {
        if (!window.sessaoId) {
            alert('Inicie uma sessão primeiro');
            return;
        }
        window.location.href = `/${window.sessaoId}/csv`;
    };
});
