import os
import shutil
from reportlab.lib.pagesizes import letter
from reportlab.lib import colors
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak, KeepTogether, HRFlowable
)
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.pdfgen import canvas

class NumberedCanvas(canvas.Canvas):
    def __init__(self, *args, **kwargs):
        super(NumberedCanvas, self).__init__(*args, **kwargs)
        self._saved_page_states = []

    def showPage(self):
        self._saved_page_states.append(dict(self.__dict__))
        self._startPage()

    def save(self):
        num_pages = len(self._saved_page_states)
        for state in self._saved_page_states:
            self.__dict__.update(state)
            self.draw_page_number(num_pages)
            canvas.Canvas.showPage(self)
        canvas.Canvas.save(self)

    def draw_page_number(self, page_count):
        self.saveState()
        self.setFont("Helvetica", 8)
        self.setFillColor(colors.HexColor("#666666"))
        
        # Header (pages > 1)
        if self._pageNumber > 1:
            self.drawString(54, 750, "METROREC - Sincronização de Vagões (LGBM) | Sistemas Operacionais 2026.2")
            self.setStrokeColor(colors.HexColor("#D0D7DE"))
            self.setLineWidth(0.5)
            self.line(54, 742, 558, 742)
        
        # Footer
        text = f"Página {self._pageNumber} de {page_count}"
        self.drawRightString(558, 36, text)
        self.drawString(54, 36, "CESAR School - Ciência da Computação")
        self.setStrokeColor(colors.HexColor("#D0D7DE"))
        self.setLineWidth(0.5)
        self.line(54, 48, 558, 48)
        self.restoreState()

def build_pdf(filename):
    doc = SimpleDocTemplate(
        filename,
        pagesize=letter,
        leftMargin=54,
        rightMargin=54,
        topMargin=54,
        bottomMargin=54
    )

    styles = getSampleStyleSheet()
    
    # Custom styles
    title_style = ParagraphStyle(
        'DocTitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=20,
        leading=24,
        textColor=colors.HexColor('#0F2C59'),
        spaceAfter=6
    )
    
    subtitle_style = ParagraphStyle(
        'DocSubtitle',
        parent=styles['Normal'],
        fontName='Helvetica-Bold',
        fontSize=12,
        leading=16,
        textColor=colors.HexColor('#337CCF'),
        spaceAfter=14
    )
    
    h1_style = ParagraphStyle(
        'Heading1_Custom',
        parent=styles['Heading1'],
        fontName='Helvetica-Bold',
        fontSize=13,
        leading=17,
        textColor=colors.HexColor('#0F2C59'),
        spaceBefore=12,
        spaceAfter=6,
        keepWithNext=True
    )
    
    h2_style = ParagraphStyle(
        'Heading2_Custom',
        parent=styles['Heading2'],
        fontName='Helvetica-Bold',
        fontSize=10.5,
        leading=14,
        textColor=colors.HexColor('#1D5D9B'),
        spaceBefore=8,
        spaceAfter=4,
        keepWithNext=True
    )
    
    body_style = ParagraphStyle(
        'Body_Custom',
        parent=styles['Normal'],
        fontName='Helvetica',
        fontSize=9.5,
        leading=13.5,
        textColor=colors.HexColor('#222222'),
        spaceAfter=6
    )
    
    bullet_style = ParagraphStyle(
        'Bullet_Custom',
        parent=body_style,
        leftIndent=15,
        firstLineIndent=-10,
        spaceAfter=4
    )

    code_style = ParagraphStyle(
        'Code_Custom',
        parent=styles['Normal'],
        fontName='Courier',
        fontSize=8,
        leading=11,
        textColor=colors.HexColor('#1A1A1A')
    )

    story = []

    # Title & Header
    story.append(Paragraph("Relatório Técnico - Sincronização de Metrô (METROREC)", title_style))
    story.append(Paragraph("Implementação 4: Concorrência e Sincronização com Pthreads", subtitle_style))
    story.append(HRFlowable(width="100%", thickness=1.5, color=colors.HexColor("#0F2C59"), spaceAfter=12))

    # Student metadata table
    meta_data = [
        [Paragraph("<b>Instituição:</b> CESAR School", body_style), Paragraph("<b>Disciplina:</b> Sistemas Operacionais (2026.2)", body_style)],
        [Paragraph("<b>Aluno:</b> Leonardo Gutzeit Borgmann Matos", body_style), Paragraph("<b>E-mail / Login:</b> lgbm@cesar.school (<b>lgbm</b>)", body_style)]
    ]
    meta_table = Table(meta_data, colWidths=[250, 254])
    meta_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor('#F5F7FA')),
        ('BOX', (0,0), (-1,-1), 1, colors.HexColor('#D0D7DE')),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor('#E1E4E8')),
        ('TOPPADDING', (0,0), (-1,-1), 5),
        ('BOTTOMPADDING', (0,0), (-1,-1), 5),
        ('LEFTPADDING', (0,0), (-1,-1), 8),
        ('RIGHTPADDING', (0,0), (-1,-1), 8),
    ]))
    story.append(meta_table)
    story.append(Spacer(1, 10))

    # Section 1
    story.append(Paragraph("1. Objetivo do Projeto", h1_style))
    story.append(Paragraph(
        "O objetivo deste projeto é projetar e implementar um mecanismo de sincronização concorrente em linguagem C "
        "para o sistema automatizado de vagões e embarque de passageiros da METROREC, derivado do clássico problema da Caltrain. "
        "Cada vagão e cada passageiro são gerenciados por threads independentes. O sistema deve atender às seguintes restrições obrigatórias:",
        body_style
    ))
    story.append(Paragraph("• Utilização estrita de primitivas POSIX Pthreads (mutex e variáveis de condição).", bullet_style))
    story.append(Paragraph("• Proibição estrita de semáforos ou outras primitivas de sincronização externas.", bullet_style))
    story.append(Paragraph("• <b>Uso de no máximo um único lock (mutex)</b> em cada <code>struct station</code>.", bullet_style))
    story.append(Paragraph("• Permitir <b>embarque simultâneo</b> (Requisito 5): diversos passageiros chamam e retornam de <code>station_wait_for_car</code> antes que qualquer um deles chame <code>station_on_board</code>.", bullet_style))
    story.append(Paragraph("• Ausência total de espera ocupada (<i>busy-waiting</i>), bloqueando threads via <code>pthread_cond_wait</code>.", bullet_style))
    story.append(Paragraph("• Automatização de compilação via Makefile, gerando o executável <code>sync_car</code>.", bullet_style))

    # Section 2
    story.append(Paragraph("2. Modelagem da Estrutura e Primitivas", h1_style))
    story.append(Paragraph(
        "Para cumprir com rigor a restrição de utilizar um único lock por estação, a estrutura <code>struct station</code> "
        "foi concebida com um único mutex e duas variáveis de condição:",
        body_style
    ))

    code_text = (
        "struct station {<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;pthread_mutex_t lock;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/* Único mutex permitido por estação */<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;pthread_cond_t train_arrived;&nbsp;&nbsp;&nbsp;/* Sinaliza chegada de vagão com vagas */<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;pthread_cond_t train_loaded;&nbsp;&nbsp;&nbsp;&nbsp;/* Sinaliza término do embarque para o vagão */<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;int waiting_passengers;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/* Passageiros aguardando na plataforma */<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;int available_seats;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/* Assentos livres no vagão presente */<br/>"
        "&nbsp;&nbsp;&nbsp;&nbsp;int boarding_passengers;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/* Passageiros autorizados em trânsito de embarque */<br/>"
        "};"
    )
    code_table = Table([[Paragraph(code_text, code_style)]], colWidths=[504])
    code_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,-1), colors.HexColor('#F6F8FA')),
        ('BOX', (0,0), (-1,-1), 0.8, colors.HexColor('#D0D7DE')),
        ('TOPPADDING', (0,0), (-1,-1), 6),
        ('BOTTOMPADDING', (0,0), (-1,-1), 6),
        ('LEFTPADDING', (0,0), (-1,-1), 10),
        ('RIGHTPADDING', (0,0), (-1,-1), 10),
    ]))
    story.append(code_table)
    story.append(Spacer(1, 8))

    story.append(Paragraph(
        "A variável <b><code>boarding_passengers</code></b> é o elemento-chave da modelagem: ela desacopla a alocação da vaga "
        "(que ocorre em <code>station_wait_for_car</code>) da confirmação de acomodação (em <code>station_on_board</code>), "
        "possibilitando que múltiplos passageiros embarquem simultaneamente sem que o trem parta prematuramente.",
        body_style
    ))

    # Section 3
    story.append(Paragraph("3. Detalhamento da Implementação", h1_style))
    story.append(Paragraph("<b>3.1 station_load_car:</b> Chamada quando o vagão atraca na estação com <code>count</code> vagas. A função armazena as vagas, emite um broadcast (<code>pthread_cond_broadcast</code>) para acordar os passageiros e aguarda em <code>pthread_cond_wait(&station->train_loaded, &station->lock)</code> enquanto houver vagas e passageiros esperando, ou enquanto houver passageiros em embarque ativo (<code>boarding_passengers > 0</code>). Ao sair do laço, zera as vagas e parte.", body_style))
    story.append(Paragraph("<b>3.2 station_wait_for_car:</b> Chamada ao passageiro chegar à estação. Incrementa <code>waiting_passengers</code> e dorme em <code>pthread_cond_wait(&station->train_arrived, &station->lock)</code> enquanto <code>available_seats == 0</code>. Ao obter vaga, decrementa <code>waiting_passengers</code> e <code>available_seats</code>, incrementa <code>boarding_passengers</code>, libera o mutex e retorna imediatamente.", body_style))
    story.append(Paragraph("<b>3.3 station_on_board:</b> Chamada quando o passageiro senta. Decrementa <code>boarding_passengers</code>. Se este for o último passageiro em trânsito (<code>boarding_passengers == 0</code>) e a condição de partida foi atingida (vagão cheio ou fila vazia), sinaliza o vagão através de <code>pthread_cond_signal(&station->train_loaded)</code>.", body_style))

    # Section 4
    story.append(Paragraph("4. Análise de Concorrência e Corretude", h1_style))
    corr_data = [
        [Paragraph("<b>Propriedade</b>", body_style), Paragraph("<b>Mecanismo de Garantia</b>", body_style)],
        [Paragraph("Ausência de Busy-Waiting", body_style), Paragraph("Bloqueio atômico de threads via <code>pthread_cond_wait</code>, sem laços vazios.", body_style)],
        [Paragraph("Embarque Simultâneo", body_style), Paragraph("O mutex é liberado antes da chamada de <code>station_on_board</code>; múltiplos passageiros retornam concorrentemente de <code>station_wait_for_car</code>.", body_style)],
        [Paragraph("Proteção contra Spurious Wakeups", body_style), Paragraph("Todas as verificações de condição utilizam laços <code>while</code> protetores.", body_style)],
        [Paragraph("Prevenção de Deadlock", body_style), Paragraph("Existe exatamente 1 mutex no sistema, eliminando qualquer grafo cíclico de espera.", body_style)],
        [Paragraph("Integridade dos Assentos", body_style), Paragraph("Alocação atômica protegida pelo mutex; nenhum vagão parte com assentos negativos.", body_style)]
    ]
    corr_table = Table(corr_data, colWidths=[160, 344])
    corr_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor('#0F2C59')),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('BOX', (0,0), (-1,-1), 0.8, colors.HexColor('#0F2C59')),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor('#D0D7DE')),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 6),
        ('RIGHTPADDING', (0,0), (-1,-1), 6),
    ]))
    for i in range(1, len(corr_data)):
        if i % 2 == 0:
            corr_table.setStyle(TableStyle([('BACKGROUND', (0, i), (-1, i), colors.HexColor('#F9FAFB'))]))
    story.append(corr_table)
    story.append(Spacer(1, 8))

    # Section 5 - Tests
    story.append(Paragraph("5. Metodologia de Testes e Resultados", h1_style))
    story.append(Paragraph(
        "Para validar a implementação, foi desenvolvida uma suíte com 7 testes cobrindo todas as rubricas no arquivo <code>main.c</code>:",
        body_style
    ))

    test_data = [
        [Paragraph("<b>ID</b>", body_style), Paragraph("<b>Cenário de Teste</b>", body_style), Paragraph("<b>Condição Avaliada</b>", body_style), Paragraph("<b>Status</b>", body_style)],
        [Paragraph("T1", body_style), Paragraph("Capacidade == Demanda", body_style), Paragraph("10 passageiros, 1 carro de 10 vagas", body_style), Paragraph("<b>PASS</b>", body_style)],
        [Paragraph("T2", body_style), Paragraph("Vagas > Demanda", body_style), Paragraph("6 passageiros, 1 carro de 15 vagas (parte quando esvazia)", body_style), Paragraph("<b>PASS</b>", body_style)],
        [Paragraph("T3", body_style), Paragraph("Vagas < Demanda", body_style), Paragraph("10 passageiros, 1 carro de 4 vagas (6 sobram na fila)", body_style), Paragraph("<b>PASS</b>", body_style)],
        [Paragraph("T4", body_style), Paragraph("Múltiplos Carros", body_style), Paragraph("15 passageiros atendidos por 3 carros (4, 5 e 6 vagas)", body_style), Paragraph("<b>PASS</b>", body_style)],
        [Paragraph("T5", body_style), Paragraph("Estação Vazia", body_style), Paragraph("0 passageiros, carro de 10 vagas parte imediatamente", body_style), Paragraph("<b>PASS</b>", body_style)],
        [Paragraph("T6", body_style), Paragraph("Embarque Simultâneo", body_style), Paragraph("5 passageiros retornam de wait_for_car antes do on_board", body_style), Paragraph("<b>PASS</b>", body_style)],
        [Paragraph("T7", body_style), Paragraph("Stress Test Concorrente", body_style), Paragraph("50 passageiros e 4 vagões sob rajadas concorrentes", body_style), Paragraph("<b>PASS</b>", body_style)]
    ]
    test_table = Table(test_data, colWidths=[30, 140, 274, 60])
    test_table.setStyle(TableStyle([
        ('BACKGROUND', (0,0), (-1,0), colors.HexColor('#0F2C59')),
        ('TEXTCOLOR', (0,0), (-1,0), colors.white),
        ('BOX', (0,0), (-1,-1), 0.8, colors.HexColor('#0F2C59')),
        ('INNERGRID', (0,0), (-1,-1), 0.5, colors.HexColor('#D0D7DE')),
        ('TOPPADDING', (0,0), (-1,-1), 4),
        ('BOTTOMPADDING', (0,0), (-1,-1), 4),
        ('LEFTPADDING', (0,0), (-1,-1), 5),
        ('RIGHTPADDING', (0,0), (-1,-1), 5),
        ('ALIGN', (0,0), (0,-1), 'CENTER'),
        ('ALIGN', (3,0), (3,-1), 'CENTER'),
    ]))
    for i in range(1, len(test_data)):
        if i % 2 == 0:
            test_table.setStyle(TableStyle([('BACKGROUND', (0, i), (-1, i), colors.HexColor('#F9FAFB'))]))
    story.append(test_table)
    story.append(Spacer(1, 8))
    story.append(Paragraph("<b>Taxa de Aprovação nos Testes:</b> 7 de 7 aprovados (100% de sucesso).", body_style))

    # Section 6 & 7
    story.append(Paragraph("6. Dificuldades Encontradas e Soluções", h1_style))
    story.append(Paragraph("<b>• Sincronização do Embarque Simultâneo:</b> A maior complexidade residiu em permitir o retorno de múltiplos passageiros em <code>station_wait_for_car</code> antes que qualquer um deles chamasse <code>station_on_board</code>, sem gerar condição de corrida na partida do trem. A solução adotada foi o contador <code>boarding_passengers</code>, assegurando que o vagão nunca parta com passageiros em trânsito.", body_style))
    story.append(Paragraph("<b>• Partida com Estação Vazia:</b> Quando um vagão chega com mais assentos que passageiros, ele deve partir assim que os passageiros presentes sentarem. Isso foi solucionado pela condição <code>(available_seats == 0 || waiting_passengers == 0) && boarding_passengers == 0</code>.", body_style))

    story.append(Paragraph("7. Conclusão", h1_style))
    story.append(Paragraph(
        "O sistema de sincronização desenvolvido para o METROREC cumpre integralmente todas as rubricas especificadas. "
        "A modelagem com apenas 1 mutex e 2 variáveis de condição demonstrou-se robusta, enxuta e eficiente, garantindo "
        "embarque simultâneo, ausência de busy-wait e total prevenção contra deadlocks e condições de corrida.",
        body_style
    ))

    doc.build(story, canvasmaker=NumberedCanvas)
    print(f"PDF successfully generated: {filename}")

if __name__ == '__main__':
    base_dir = os.path.dirname(os.path.abspath(__file__))
    out_pdf = os.path.join(base_dir, 'lgbm.pdf')
    build_pdf(out_pdf)
    
    # Also copy to root directory for maximum convenience
    root_dir = os.path.dirname(base_dir)
    root_pdf = os.path.join(root_dir, 'lgbm.pdf')
    shutil.copy2(out_pdf, root_pdf)
    print(f"Copied to root: {root_pdf}")
