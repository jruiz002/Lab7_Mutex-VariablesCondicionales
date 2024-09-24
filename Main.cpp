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

// Función para calcular el monto de venta por producto
double calcularMontoVentas(int unidadesVendidas, double precioUnitario) {
    return unidadesVendidas * precioUnitario;
}

// Función para calcular la utilidad por producto
double calcularUtilidadPorProducto(double ventasProducto, int unidadesVendidas, double costoUnitario) {
    return ventasProducto - (unidadesVendidas * costoUnitario);
}

// Variables globales
Producto* productosMes;
int numProductos;
pthread_mutex_t candado = PTHREAD_MUTEX_INITIALIZER;   // Mutex para proteger acceso a los datos
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;      // Variable condicional
int hilosTerminados = 0;                             // Contador de hilos terminados

// Función que será ejecutada por cada hilo
void* calcularVentasYUtilidades(void* arg) {
    int idx = *(int*)arg;

    // Imprimir cuando el hilo inicia
    cout << "Cálculo hilo " << idx << " iniciado" << endl;

    // Simular el tiempo de cálculo
    sleep(1);

    // Calcular ventas y utilidades del producto
    double ventas = calcularMontoVentas(productosMes[idx].cantidadVendida, productosMes[idx].precioUnitario);
    double utilidad = calcularUtilidadPorProducto(ventas, productosMes[idx].cantidadVendida, productosMes[idx].costoUnitario);

    // Bloquear el mutex antes de modificar los datos compartidos
    pthread_mutex_lock(&candado);

    productosMes[idx].ventasProducto = ventas;
    productosMes[idx].utilidadProducto = utilidad;
    
    // Incrementar el número de hilos que han terminado
    hilosTerminados++;

    // Imprimir cuando el hilo termina
    cout << "Cálculo hilo " << idx << " terminado" << endl;

    // Si todos los hilos han terminado, señalizar la condición
    if (hilosTerminados == numProductos) {
        pthread_cond_signal(&cond);
    }

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
void procesarMes(string nombreMes, Producto productos[], int numProd) {
    productosMes = productos;
    numProductos = numProd;
    hilosTerminados = 0;

    // Crear un hilo por cada producto
    pthread_t hilos[numProductos];
    int indices[numProductos];

    for (int i = 0; i < numProductos; ++i) {
        indices[i] = i;
        pthread_create(&hilos[i], NULL, calcularVentasYUtilidades, &indices[i]);
    }

    // Bloquear el mutex para esperar la señal de la variable condicional
    pthread_mutex_lock(&candado);

    // Esperar a que todos los hilos terminen
    while (hilosTerminados < numProductos) {
        pthread_cond_wait(&cond, &candado);
    }

    // Desbloquear el mutex una vez que todos los hilos hayan terminado
    pthread_mutex_unlock(&candado);

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

int main() {
    // Datos de los productos para julio
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

    // Datos de los productos para agosto
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

    // Procesar julio
    procesarMes("JULIO", productosJulio, 8);

    // Procesar agosto
    procesarMes("AGOSTO", productosAgosto, 8);

    // Destruir los mutex y variables condicionales
    pthread_mutex_destroy(&candado);
    pthread_cond_destroy(&cond);

    return 0;
}
