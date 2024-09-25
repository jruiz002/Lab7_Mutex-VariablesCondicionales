#include <iostream>
#include <pthread.h>
#include <unistd.h> // Para usar sleep()

using namespace std;

// Estructura para almacenar la información de cada producto
struct Producto {
    string nombre;
    double precioUnitario;
    double costoUnitario;
    int cantidadVendida;
    double ventasProducto;
    double utilidadProducto;
};

// Variables globales
Producto* productosMes;
int numProductos;
pthread_mutex_t candado = PTHREAD_MUTEX_INITIALIZER;  
int hilosTerminados = 0;                           

// Función que será ejecutada por cada hilo
void* calcularVentasYUtilidades(void* arg) {
    int idx = *(int*)arg;

    cout << "Cálculo hilo " << idx << " iniciado" << endl;

    // Simular el tiempo de cálculo
    sleep(1);

    double ventas = productosMes[idx].cantidadVendida * productosMes[idx].precioUnitario;
    double utilidad = ventas - (productosMes[idx].cantidadVendida * productosMes[idx].costoUnitario);

    // Bloquear el mutex antes de modificar los datos compartidos
    pthread_mutex_lock(&candado);

    productosMes[idx].ventasProducto = ventas;
    productosMes[idx].utilidadProducto = utilidad;
    
    hilosTerminados++;

    cout << "Cálculo hilo " << idx << " terminado" << endl;

    // Desbloquear el mutex después de modificar los datos
    pthread_mutex_unlock(&candado);

    return NULL;
}

// Función para calcular el monto total de ventas del mes
double calcularMontoTotalVentas() {
    double totalVentas = 0;
    for (int i = 0; i < numProductos; ++i) {
        totalVentas += productosMes[i].ventasProducto;
    }
    return totalVentas;
}

// Función para calcular la utilidad total del mes
double calcularUtilidadTotalMes() {
    double totalUtilidad = 0;
    for (int i = 0; i < numProductos; ++i) {
        totalUtilidad += productosMes[i].utilidadProducto;
    }
    return totalUtilidad;
}

// Función para procesar los productos de un mes
void procesarMes(string nombreMes) {
    // Esperar a que todos los hilos terminen antes de imprimir
    while (hilosTerminados < numProductos) {
        sleep(1);
    }

    // Imprimir el reporte del mes
    cout << "------------------------------------------" << endl;
    cout << "REPORTE DEL MES DE " << nombreMes << endl;
    cout << "--- Monto Ventas por Producto ---" << endl;
    for (int i = 0; i < numProductos; ++i) {
        cout << "- " << productosMes[i].nombre << ": Q" << productosMes[i].ventasProducto << endl;
    }

    cout << "--- Utilidad por Producto ---" << endl;
    for (int i = 0; i < numProductos; ++i) {
        cout << "- " << productosMes[i].nombre << ": Q" << productosMes[i].utilidadProducto << endl;
    }

    double totalVentas = calcularMontoTotalVentas();
    double totalUtilidad = calcularUtilidadTotalMes();

    cout << "--- Monto Total Ventas del Mes: Q" << totalVentas << endl;
    cout << "--- Utilidad del mes: Q" << totalUtilidad << endl;
    cout << "------------------------------------------" << endl;
}

void procesarProductos(Producto productos[], int numProd, string nombreMes) {
    productosMes = productos;
    numProductos = numProd;
    hilosTerminados = 0;  // Reiniciar contador de hilos terminados

    // Crear un hilo por cada producto
    pthread_t hilos[numProductos];
    int indices[numProductos];

    for (int i = 0; i < numProductos; ++i) {
        indices[i] = i;
        pthread_create(&hilos[i], NULL, calcularVentasYUtilidades, &indices[i]);
    }

    // Procesar el mes (esperar a que los hilos terminen e imprimir)
    procesarMes(nombreMes);

    // Unir todos los hilos (esperar a que todos los hilos finalicen)
    for (int i = 0; i < numProductos; ++i) {
        pthread_join(hilos[i], NULL);
    }
}

int main() {
    // Datos de los productos para los meses
    Producto productosJulio[] = {
        {"Porción pastel de chocolate", 60, 20, 300},
        {"White mocha", 32, 19.20, 400},
        {"Café americano 8onz", 22, 13.20, 1590},
        {"Latte 8onz", 24, 17.2, 200},
        {"Toffee coffee",28,  20.1, 390},
        {"Cappuccino 8onz",24, 17.2, 1455},
        {"S'mores Latte ",32, 23, 800},
        {"Café tostado molido",60, 20, 60}
    };

    Producto productosAgosto[] = {
        {"Porción pastel de chocolate", 60, 20, 250},
        {"White mocha", 32, 19.20, 380},
        {"Café americano 8onz", 22, 13.20, 800},
        {"Latte 8onz", 24, 17.2, 250},
        {"Toffee coffee",28,  20.1, 600},
        {"Cappuccino 8onz",24, 17.2, 1200},
        {"S'mores Latte ",32, 23, 1540},
        {"Café tostado molido",60, 20, 15}
    };

    // Procesar los productos de Julio
    procesarProductos(productosJulio, 8, "JULIO");

    // Procesar los productos de Agosto
    procesarProductos(productosAgosto, 8, "AGOSTO");

    // Destruir el mutex
    pthread_mutex_destroy(&candado);

    return 0;
}