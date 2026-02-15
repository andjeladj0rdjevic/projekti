import numpy as np
import matplotlib.pyplot as plt

# PARAMETRI MREŽE

Lx = 1.0
Ly = 1.0
dx = 0.2
dy = 0.2

nx = int(Lx/dx) + 1
ny = int(Ly/dy) + 1

eps = 0.01
max_iter = 10000


# FUNKCIJA ZA POSTAVLJANJE GRANIČNIH USLOVA

def apply_boundary_conditions(psi):

    # Donji zid
    psi[:, 0] = 0

    # Gornji zid
    psi[:, -1] = 100

    # Levi zid
    psi[0, :] = np.linspace(0, 100, ny)

    # Desni zid (dψ/dx = 0)
    psi[-1, :] = psi[-2, :]

    return psi


# GAUSS-SEIDEL + RELAKSACIJA

def gauss_seidel(omega, psi_initial):

    psi = psi_initial.copy()
    iteration = 0

    while iteration < max_iter:

        psi_old = psi.copy()

        for i in range(1, nx-1):
            for j in range(1, ny-1):

                psi[i,j] = (1 - omega)*psi[i,j] + omega*0.25*(
                    psi[i+1,j] + psi[i-1,j] +
                    psi[i,j+1] + psi[i,j-1]
                )

        psi = apply_boundary_conditions(psi)

        error = np.max(np.abs(psi - psi_old))

        if error < eps:
            break

        iteration += 1

    return psi, iteration


# ADI METOD

def adi_method(psi_initial):

    psi = psi_initial.copy()
    iteration = 0

    while iteration < max_iter:

        psi_old = psi.copy()

        # implicitno po x
        for j in range(1, ny-1):
            for i in range(1, nx-1):
                psi[i,j] = 0.25*(
                    psi[i+1,j] + psi[i-1,j] +
                    psi[i,j+1] + psi[i,j-1]
                )

        # implicitno po y
        for i in range(1, nx-1):
            for j in range(1, ny-1):
                psi[i,j] = 0.25*(
                    psi[i+1,j] + psi[i-1,j] +
                    psi[i,j+1] + psi[i,j-1]
                )

        psi = apply_boundary_conditions(psi)

        error = np.max(np.abs(psi - psi_old))

        if error < eps:
            break

        iteration += 1

    return psi, iteration


# OPTIMALNI OMEGA

omegas = np.linspace(1.0, 1.9, 10)
iterations_list = []

for omega in omegas:

    psi0 = np.zeros((nx, ny))
    psi0 = apply_boundary_conditions(psi0)

    _, it = gauss_seidel(omega, psi0)
    iterations_list.append(it)

# Grafik iteracija vs omega
plt.figure()
plt.plot(omegas, iterations_list, marker='o')
plt.xlabel("Omega")
plt.ylabel("Broj iteracija")
plt.title("Konvergencija u funkciji omega")
plt.grid()
plt.show()

optimal_omega = omegas[np.argmin(iterations_list)]
print("Optimalni omega:", optimal_omega)


# UTICAJ POČETNOG REŠENJA

initial_values = [0, 25, 50, 100]
iterations_initial = []

for val in initial_values:

    psi0 = np.full((nx, ny), val)
    psi0 = apply_boundary_conditions(psi0)

    _, it = gauss_seidel(optimal_omega, psi0)
    iterations_initial.append(it)

plt.figure()
plt.plot(initial_values, iterations_initial, marker='o')
plt.xlabel("Početna vrednost psi")
plt.ylabel("Broj iteracija")
plt.title("Uticaj početnog rešenja")
plt.grid()
plt.show()


# FINALNO REŠENJE (GS)

psi0 = np.zeros((nx, ny))
psi0 = apply_boundary_conditions(psi0)

psi_final, _ = gauss_seidel(optimal_omega, psi0)

x = np.linspace(0, Lx, nx)
y = np.linspace(0, Ly, ny)
X, Y = np.meshgrid(x, y)

plt.figure()
plt.contourf(X, Y, psi_final.T, 20)
plt.colorbar(label="Psi")
plt.title("Strujne linije - GS")
plt.xlabel("x")
plt.ylabel("y")
plt.show()


# FINALNO REŠENJE (ADI)

psi0 = np.zeros((nx, ny))
psi0 = apply_boundary_conditions(psi0)

psi_adi, _ = adi_method(psi0)

plt.figure()
plt.contourf(X, Y, psi_adi.T, 20)
plt.colorbar(label="Psi")
plt.title("Strujne linije - ADI")
plt.xlabel("x")
plt.ylabel("y")
plt.show()

