document.addEventListener('DOMContentLoaded', function () {
    const cycleForm      = document.getElementById('cycleForm');
    const btnDownload    = document.getElementById('btnDownload');
    const btnStop        = document.querySelector('.btn-stop');
    const timerDisplay   = document.getElementById('timerDisplay');

    let timerIntervalId = null;
    let segsDecorridos  = 0;
    let duracaoTotalSeg = 0;

    function formatarTempo(segs) {
        const minutos = Math.floor(segs / 60);
        const segundos = segs % 60;
        return `${minutos.toString().padStart(2, '0')}:${segundos.toString().padStart(2, '0')}`;
    }

    function iniciarCronometro(duracao) {
        segsDecorridos  = 0;
        duracaoTotalSeg = duracao * 60;
        timerDisplay.textContent = '00:00';
        timerDisplay.removeAttribute('hidden');

        if (timerIntervalId) clearInterval(timerIntervalId);

        timerIntervalId = setInterval(() => {
            segsDecorridos++;
            timerDisplay.textContent = formatarTempo(segsDecorridos);
            if (segsDecorridos >= duracaoTotalSeg) {
                pararCronometro();
            }
        }, 1000);
    }

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

        const duracao     = parseInt(document.getElementById('duracao').value, 10);
        const amostra     = document.getElementById('amostra').value;
        const descricao   = document.getElementById('description').value;
        const temperatura = parseFloat(document.getElementById('temperatura').value);

        if (sensores.length === 0) {
            alert('Selecione pelo menos um sensor.');
            return;
        }

        btnDownload.disabled = true;
        btnDownload.style.opacity = 0.5;

        iniciarCronometro(duracao);

        try {
            const response = await fetch('/iniciar', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    sessaoRequest: {
                        amostra,
                        descricao,
                        sensores
                    },
                    cicloRequest: {
                        duracao,
                        temperatura
                    }
                })
            });

            const data = await response.json();

            if (response.ok) {
                alert(data.mensagem);
                window.sessaoId = data.id;

                setTimeout(() => {
                    pararCronometro();
                    btnDownload.disabled = false;
                    btnDownload.style.opacity = 1;
                }, duracao * 60 * 1000);
            } else {
                alert(data.erro || 'Erro ao iniciar sessão');
                pararCronometro();
                timerDisplay.setAttribute('hidden', 'hidden');
            }

        } catch (err) {
            alert('Erro na comunicação com o servidor');
            pararCronometro();
            timerDisplay.setAttribute('hidden', 'hidden');
        }
    });

    btnStop.addEventListener('click', async () => {
        if (!window.sessaoId) {
            alert('Nenhuma sessão ativa.');
            return;
        }

        try {
            const response = await fetch(`/encerrar/${window.sessaoId}`, {
                method: 'POST'
            });

            const data = await response.json();

            if (response.ok) {
                alert(data.mensagem);
                pararCronometro();
                btnDownload.disabled = false;
                btnDownload.style.opacity = 1;
            } else {
                alert(data.erro || 'Erro ao encerrar sessão');
            }

        } catch (err) {
            alert('Erro na comunicação com o servidor');
        }
    });

    window.downloadCSV = function () {
        if (!window.sessaoId) {
            alert('Inicie uma sessão primeiro.');
            return;
        }
        window.location.href = `/${window.sessaoId}/csv`;
    };
});
