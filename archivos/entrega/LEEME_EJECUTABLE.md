# Ejecutable de entrega

Archivo principal:

- `ClavadosCiudadAcademia.exe`

Este ejecutable fue generado desde la configuracion Release de Qt/MinGW.

## Nota importante

El archivo `.exe` queda incluido en el repositorio para cumplir la entrega. Si se va a ejecutar en un computador que no tenga Qt instalado, se recomienda generar un paquete desplegable con `windeployqt`.

Comando sugerido desde la raiz del repositorio:

```powershell
codigo\scripts\preparar_entrega.ps1
```

Ese script recompila el proyecto, copia el `.exe` y ejecuta `windeployqt` sobre una carpeta local de entrega.
