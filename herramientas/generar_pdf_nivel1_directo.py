from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import inch
from reportlab.platypus import (
    ListFlowable,
    ListItem,
    PageBreak,
    Paragraph,
    SimpleDocTemplate,
    Spacer,
    Table,
    TableStyle,
)


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "archivos" / "sustentacion"
OUT_DIR.mkdir(parents=True, exist_ok=True)
PDF_PATH = OUT_DIR / "Explicacion_Nivel_1_Fisicas.pdf"


def styles():
    base = getSampleStyleSheet()
    base["Normal"].fontName = "Helvetica"
    base["Normal"].fontSize = 9.2
    base["Normal"].leading = 11.5
    base["Normal"].spaceAfter = 4
    base.add(
        ParagraphStyle(
            "TitleMain",
            parent=base["Title"],
            fontName="Helvetica-Bold",
            fontSize=20,
            leading=24,
            alignment=TA_CENTER,
            textColor=colors.HexColor("#1F4D78"),
            spaceAfter=8,
        )
    )
    base.add(
        ParagraphStyle(
            "Subtitle",
            parent=base["Normal"],
            fontSize=11,
            leading=14,
            alignment=TA_CENTER,
            textColor=colors.HexColor("#3E5C70"),
            spaceAfter=14,
        )
    )
    base.add(
        ParagraphStyle(
            "H1Custom",
            parent=base["Heading1"],
            fontName="Helvetica-Bold",
            fontSize=14,
            leading=17,
            textColor=colors.HexColor("#2E74B5"),
            spaceBefore=10,
            spaceAfter=7,
        )
    )
    base.add(
        ParagraphStyle(
            "H2Custom",
            parent=base["Heading2"],
            fontName="Helvetica-Bold",
            fontSize=11.5,
            leading=14,
            textColor=colors.HexColor("#1F4D78"),
            spaceBefore=8,
            spaceAfter=5,
        )
    )
    base.add(
        ParagraphStyle(
            "Small",
            parent=base["Normal"],
            fontSize=8.2,
            leading=10,
        )
    )
    base.add(
        ParagraphStyle(
            "CodeSmall",
            parent=base["Normal"],
            fontName="Courier",
            fontSize=8,
            leading=9.5,
            textColor=colors.HexColor("#1D2935"),
        )
    )
    return base


def p(text, style):
    text = (
        text.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace("\n", "<br/>")
    )
    return Paragraph(text, style)


def note(text, st):
    table = Table([[p("<b>Nota:</b> " + text, st["Normal"])]], colWidths=[7.0 * inch])
    table.setStyle(
        TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, -1), colors.HexColor("#EAF3F8")),
                ("BOX", (0, 0), (-1, -1), 0.6, colors.HexColor("#8BB8D8")),
                ("LEFTPADDING", (0, 0), (-1, -1), 8),
                ("RIGHTPADDING", (0, 0), (-1, -1), 8),
                ("TOPPADDING", (0, 0), (-1, -1), 6),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 6),
            ]
        )
    )
    return table


def table(data, widths, st, header=True):
    converted = []
    for r, row in enumerate(data):
        converted.append([p(str(cell), st["Small"]) for cell in row])
    t = Table(converted, colWidths=[w * inch for w in widths], repeatRows=1 if header else 0)
    commands = [
        ("GRID", (0, 0), (-1, -1), 0.35, colors.HexColor("#B9C7D3")),
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("LEFTPADDING", (0, 0), (-1, -1), 5),
        ("RIGHTPADDING", (0, 0), (-1, -1), 5),
        ("TOPPADDING", (0, 0), (-1, -1), 4),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
    ]
    if header:
        commands += [
            ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#E8EEF5")),
            ("TEXTCOLOR", (0, 0), (-1, 0), colors.HexColor("#1F4D78")),
        ]
    t.setStyle(TableStyle(commands))
    return t


def build_pdf():
    st = styles()
    doc = SimpleDocTemplate(
        str(PDF_PATH),
        pagesize=letter,
        leftMargin=0.55 * inch,
        rightMargin=0.55 * inch,
        topMargin=0.55 * inch,
        bottomMargin=0.55 * inch,
        title="Explicacion Nivel 1 y Fisicas",
    )
    story = []
    story.append(p("Clavados en Ciudad Academia", st["TitleMain"]))
    story.append(p("Explicacion linea a linea del Nivel 1 y sus fisicas", st["Subtitle"]))
    story.append(
        note(
            "Usa este PDF como guia de estudio para sustentar el archivo NivelPiscinaEntrenamiento.cpp. "
            "Los rangos de lineas corresponden al archivo actual del repositorio.",
            st,
        )
    )
    story.append(Spacer(1, 8))

    story.append(p("Resumen rapido", st["H1Custom"]))
    bullets = [
        "El Nivel 1 es una caida desde plataforma hacia una piscina movil.",
        "La posicion se calcula con velocidad y aceleracion usando dt.",
        "Las fisicas defendibles son gravedad, viento turbulento, piscina con aceleracion variable y campos de monedas.",
        "La dificultad cambia parametros fisicos y reglas de evaluacion.",
        "La memoria dinamica se ve en make_unique y contenedores de unique_ptr.",
    ]
    story.append(
        ListFlowable(
            [ListItem(p(item, st["Normal"])) for item in bullets],
            bulletType="bullet",
            leftIndent=18,
        )
    )

    story.append(p("Fisicas que debes sustentar", st["H1Custom"]))
    physics = [
        ["Fisica", "Ecuacion / regla", "Defensa oral"],
        [
            "Gravedad / parabola",
            "ay = gravedad * factorGravedad; vy += ay * dt; y += vy * dt",
            "La velocidad vertical cambia con el tiempo. El salto inicia con vy negativa y la gravedad la vuelve positiva.",
        ],
        [
            "Viento turbulento",
            "viento = vientoBase + sin(tiempo * 4 + y * 0.01) * 26",
            "Tiene parte constante y parte sinusoidal para simular rafagas.",
        ],
        [
            "Piscina movil",
            "a = sin(tiempo * 0.82) * 34 * factor; v += a * dt; x += v * dt",
            "La piscina acelera y frena; no se mueve solo con velocidad constante.",
        ],
        [
            "Campo Mikoto",
            "distancia = sqrt(dx^2 + dy^2); fuerza depende de radio e influencia",
            "Las monedas aceleran hacia el jugador segun distancia.",
        ],
        [
            "Accelerator",
            "vx/vy se mezclan con velocidad objetivo hacia el personaje",
            "Controla directamente el vector velocidad de la moneda.",
        ],
        [
            "Mugino",
            "Haz lateral dentro de un corredor vertical",
            "Canaliza monedas con una fisica direccional, no radial.",
        ],
        [
            "Dark Matter",
            "fuerza radial + fuerza tangencial perpendicular",
            "Produce curvatura/orbita alrededor del campo.",
        ],
    ]
    story.append(table(physics, [1.35, 2.65, 3.0], st))

    story.append(PageBreak())
    story.append(p("Explicacion linea a linea por bloques", st["H1Custom"]))
    story.append(
        note(
            "Si el profesor pide una linea concreta, ubica el rango. Si pregunta por fisicas, ve directo a 135-174, 195-254 y 650-737.",
            st,
        )
    )
    rows = [
        ["Lineas", "Bloque", "Explicacion para sustentar"],
        ["1-5", "Includes", "Importan la cabecera del nivel, cache de sprites, algoritmos, matematicas y gradientes."],
        ["7-18", "Panel HUD", "Dibuja paneles del HUD. Es capa grafica, no fisica."],
        ["20-28", "Barras", "Dibuja barras de progreso; limita el porcentaje entre 0 y 1."],
        ["30-33", "Hitbox", "Contrae rectangulos de colision para que sean mas justos."],
        ["36-40", "Constructor", "Crea jugador, plataforma y dron con make_unique. Sustenta memoria dinamica."],
        ["41-46", "Mundo fisico", "Define altura del mundo, camara, piscina, zona meta, viento y suelo."],
        ["48-71", "Sprites", "Carga recursos visuales del nivel."],
        ["73-92", "Estado inicial", "Inicializa puntaje, banderas, viento, gravedad, poder y piscina."],
        ["94-99", "Dificultad", "Configura normal, aplica parametros, ubica plataforma y crea monedas."],
        ["101-104", "Posicion inicial", "Ubica al jugador sobre la plataforma calculando centro y altura."],
        ["107", "Destructor", "Destructor por defecto; unique_ptr libera memoria automaticamente."],
        ["109-126", "Parametros", "Toma viento, energia y gravedad segun dificultad."],
        ["128-133", "Cambiar dificultad", "Reconfigura parametros y reinicia el nivel."],
        ["135-146", "Update inicial", "Actualiza plataforma, dron, tiempo, cooldown, piscina y monedas."],
        ["148-153", "Plataforma", "Si no esta en el aire, el jugador se mantiene sobre la plataforma."],
        ["155-159", "Gravedad", "Aplica aceleracion vertical al jugador. Esta es fisica clave."],
        ["160-163", "Turbulencia", "Calcula rafaga con seno y la aplica como viento lateral."],
        ["165-167", "Dron empuja", "El dron aplica empuje lateral si intersecta con el jugador."],
        ["169-173", "Integracion", "Actualiza jugador, verifica colisiones y mueve camara."],
        ["176-193", "Crear monedas", "Limpia vector, reserva memoria y crea monedas dinamicas."],
        ["195-202", "Distancia", "Calcula dx, dy y distancia euclidiana entre moneda y jugador."],
        ["204-217", "Poderes", "Aplica modelo fisico distinto segun personaje."],
        ["220-225", "Recolectar", "Usa hitbox contraida, suma puntaje y sonido."],
        ["229-233", "Piscina pausada", "No actualiza piscina si el intento ya termino."],
        ["235-240", "Piscina acelerada", "Usa seno para aceleracion, integra velocidad y desplaza x."],
        ["242-253", "Limites", "Rebota la piscina en bordes y mueve zona meta con ella."],
        ["256-259", "Camara", "Sigue al jugador con clamp para no salir del mundo."],
        ["261-264", "Colisiones", "Agrupa verificacion de piscina y suelo."],
        ["267-273", "Zona viento", "Detecta si el jugador esta dentro del area de viento."],
        ["276-308", "Piscina", "Al entrar cayendo, calcula puntaje, victoria, sonido y aprendizaje del dron."],
        ["311-329", "Suelo", "Si cae fuera de piscina, pierde intento y se detiene."],
        ["332-379", "Puntaje", "Evalua error al centro, velocidad vertical y monedas."],
        ["381-398", "Fondo", "Dibuja fondo vertical o gradiente alternativo."],
        ["400-418", "Viento y piso", "Dibuja rafagas y piso final."],
        ["420-437", "Piscina visual", "Dibuja decoracion y sprite de piscina segun posicion fisica."],
        ["439-448", "Entidades", "Dibuja monedas, plataforma, dron y poder."],
        ["450-482", "Splash", "Dibuja entrada al agua, burbujas y splash segun calidad."],
        ["485-508", "Rafagas", "Animacion visual con seno, coherente con turbulencia."],
        ["510-571", "Poder visual", "Dibuja vectorial, haz, orbita o campo radial segun personaje."],
        ["573-638", "HUD", "Muestra energia, puntaje, monedas, intentos y estado."],
        ["640-648", "Render", "Aplica camara al mundo y deja HUD fijo."],
        ["650-658", "Salto", "Espacio asigna vx=130 y vy=-86; luego gravedad forma parabola."],
        ["661-667", "A/D", "Activa aceleracion lateral en Personaje."],
        ["669-692", "Acciones", "E poder, R reinicio, 1/2/3 dificultad."],
        ["695-704", "Liberar teclas", "Desactiva movimiento lateral al soltar A/D."],
        ["706-710", "Mouse", "Click activa poder."],
        ["712-737", "Poder", "Define duracion, cooldown y radio segun personaje."],
        ["739-769", "Reiniciar intento", "Restaura estado parcial, monedas, piscina, dron y jugador."],
        ["771-807", "Reiniciar nivel", "Restaura todo el nivel, energia, intentos, camara y monedas."],
        ["809-814", "Personaje", "Configura tipo, radio de poder y reinicia."],
        ["816-840", "Consultas", "Retorna puntaje, estado del nivel, nombre y sonidos."],
    ]
    story.append(table(rows, [0.72, 1.45, 4.83], st))

    story.append(PageBreak())
    story.append(p("Guion oral de 60 segundos", st["H1Custom"]))
    story.append(
        p(
            "En el Nivel 1 el jugador salta desde una plataforma hacia una piscina movil. "
            "La fisica principal es una integracion por dt: primero se acumulan aceleraciones, "
            "luego se actualizan velocidades y por ultimo posiciones. La gravedad produce una "
            "trayectoria parabolica porque el jugador inicia con velocidad vertical negativa y "
            "luego la aceleracion de gravedad cambia esa velocidad. Tambien hay una zona de viento "
            "con turbulencia sinusoidal, una piscina con aceleracion variable y poderes que modifican "
            "la velocidad de las monedas. La dificultad cambia parametros fisicos y el dron funciona "
            "como agente inteligente porque percibe, razona, actua y aprende del error de entrada.",
            st["Normal"],
        )
    )

    story.append(p("Preguntas duras", st["H1Custom"]))
    qa = [
        ["Pregunta", "Respuesta corta"],
        ["Por que no es movimiento rectilineo simple?", "Porque hay aceleracion: la velocidad cambia por gravedad, viento y campos."],
        ["Donde esta dt?", "En actualizar(dt): escala velocidad, aceleracion, cooldowns y movimiento."],
        ["Fisica mas clara?", "Gravedad: ay = gravedad * factor, vy += ay*dt, y += vy*dt."],
        ["Dificultad no trivial?", "Cambia viento, energia, intentos, puntaje minimo, gravedad y movimiento."],
        ["Memoria dinamica?", "make_unique y unique_ptr para jugador, plataforma, dron y monedas."],
        ["POO?", "Entidad define actualizar/dibujar; cada clase especializa comportamiento."],
    ]
    story.append(table(qa, [2.2, 4.8], st))

    doc.build(story)
    return PDF_PATH


if __name__ == "__main__":
    print(build_pdf())
