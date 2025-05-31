document.addEventListener('DOMContentLoaded', function() {
    const cycleForm = document.getElementById('cycleForm');
    const btnDownload = document.getElementById('btnDownload');

    // Função para iniciar a coleta
    cycleForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        const sensores = Array.from(document.querySelectorAll('input[name="sensors"]:checked')).map(el => el.value);
        const duracao = document.getElementById('duracao').value;

        if (sensores.length === 0) {
            alert('Selecione pelo menos um sensor');
            return;
        }

        // Desabilita o botão de download ao iniciar a coleta
        btnDownload.disabled = true;
        btnDownload.style.opacity = 0.5;

        const response = await fetch('/iniciar', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                usuario: 1, // Ajuste para o ID do usuário logado
                sensores,
                duracao: parseInt(duracao)
            })
        });

        const data = await response.json();
        if (response.ok) {
            alert(data.mensagem);
            window.sessaoId = data.id; // Salva o ID da sessão para usar no download

            // Inicia a verificação periódica do status do CSV
            iniciarVerificacaoCsv(data.id, parseInt(duracao));
        } else {
            alert(data.erro || 'Erro ao iniciar sessão');
        }
    });

    // Função para verificar se o CSV está pronto
    async function verificarCsvPronto(sessaoId) {
        try {
            const response = await fetch(`/${sessaoId}/csv/status`);
            if (response.ok) {
                const data = await response.json();
                if (data.pronto) {
                    // Habilita o botão de download
                    btnDownload.disabled = false;
                    btnDownload.style.opacity = 1;
                    clearInterval(window.intervalId); // Limpa o intervalo
                }
            } else {
                console.error('Erro ao verificar status do CSV');
            }
        } catch (error) {
            console.error('Erro na requisição de status do CSV:', error);
        }
    }

    // Função para iniciar a verificação periódica
    function iniciarVerificacaoCsv(sessaoId, duracaoMinutos) {
        const delayMs = duracaoMinutos * 60 * 1000;
        window.intervalId = setInterval(() => verificarCsvPronto(sessaoId), 5000); // Verifica a cada 5 segundos
    }

    // Função para realizar o download do CSV
    window.downloadCSV = function() {
        if (!window.sessaoId) {
            alert('Inicie uma sessão primeiro');
            return;
        }
        window.location.href = `/sessao/${window.sessaoId}/csv`;
    };
});