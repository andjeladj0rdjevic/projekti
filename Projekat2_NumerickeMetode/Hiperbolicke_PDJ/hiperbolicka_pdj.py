import numpy as np
import matplotlib.pyplot as plt

# ------------------ PARAMETRI ------------------
L = 4.0         # duzina domena (uzimamo do x=4 da pokrijemo i deo gde je 2(x-2))
T = 0.3         # vreme simulacije
dx = 0.2        # korak u prostoru (iz zadatka)
dt = 0.1        # korak u vremenu (iz zadatka)
a = 1.0         # brzina (iz PDJ: Ut + Ux = 0, gde je koeficijent uz Ux jednak 1)

# ------------------ IZRACUNAVANJE BROJA TACAKA ------------------
Nx = int(L / dx) + 1        # broj tacaka u prostoru
Nt = int(T / dt) + 1        # broj vremenskih koraka

# ------------------ CFL USLOV ------------------
C = a * dt / dx
print(f"Parametri simulacije:")
print(f"  dx = {dx}")
print(f"  dt = {dt}")
print(f"  Nx = {Nx} (broj prostornih tacaka)")
print(f"  Nt = {Nt} (broj vremenskih koraka)")
print(f"  CFL broj (C) = {C:.3f}")
print()

if C > 1.0:
    print("UPOZORENJE: Nestabilno! CFL > 1. Potrebno je smanjiti dt ili povecati dx.")
    print()
else:
    print("Status: Stabilno (CFL <= 1)")
    print()

# ------------------ MREZE ------------------
x = np.linspace(0, L, Nx)
t = np.linspace(0, T, Nt)

# ------------------ POCETNI USLOV ------------------
# U(x,0) = x(x-2),     0 <= x <= 2
# U(x,0) = 2(x-2),     2 <= x
U0 = np.zeros_like(x)

for i in range(Nx):
    if x[i] <= 2.0:
        U0[i] = x[i] * (x[i] - 2.0)
    else:
        U0[i] = 2.0 * (x[i] - 2.0)

# ------------------ MATRICE RESENJA ------------------
U_lw = np.zeros((Nx, Nt))        # Lax-Wendroff
U_mc = np.zeros((Nx, Nt))        # MacCormack
U_cn = np.zeros((Nx, Nt))        # Crank-Nicolson

# Pocetni uslov (t=0)
U_lw[:, 0] = U0.copy()
U_mc[:, 0] = U0.copy()
U_cn[:, 0] = U0.copy()

# Granični uslov na levoj granici: U(0,t) = 2t
for j in range(Nt):
    U_lw[0, j] = 2.0 * t[j]
    U_mc[0, j] = 2.0 * t[j]
    U_cn[0, j] = 2.0 * t[j]


# =========================================================
# (a) LAX-WENDROFF METODA
# =========================================================
print("Računam Lax-Wendroff metodu...")
for j in range(Nt - 1):
    for i in range(1, Nx - 1):
        U_lw[i, j + 1] = (U_lw[i, j]
                          - (C / 2.0) * (U_lw[i + 1, j] - U_lw[i - 1, j])
                          + (C**2 / 2.0) * (U_lw[i + 1, j] - 2.0 * U_lw[i, j] + U_lw[i - 1, j]))
    
    # Desna granica (outflow/ekstrapolacija)
    U_lw[Nx - 1, j + 1] = U_lw[Nx - 2, j + 1]


# =========================================================
# (b) MACCORMACK METODA
# =========================================================
print("Računam MacCormack metodu...")
for j in range(Nt - 1):
    # PREDIKTOR KORAK
    U_pred = np.zeros(Nx)
    U_pred[0] = U_mc[0, j + 1]  # leva granica
    
    for i in range(1, Nx):
        U_pred[i] = U_mc[i, j] - C * (U_mc[i, j] - U_mc[i - 1, j])
    
    # Desna granica u prediktoru
    U_pred[Nx - 1] = U_pred[Nx - 2]
    
    # KOREKTOR KORAK
    for i in range(1, Nx - 1):
        U_mc[i, j + 1] = 0.5 * (U_mc[i, j] + U_pred[i]
                                - C * (U_pred[i + 1] - U_pred[i]))
    
    # Desna granica (outflow)
    U_mc[Nx - 1, j + 1] = U_mc[Nx - 2, j + 1]


# =========================================================
# (c) CRANK-NICOLSON METODA
# =========================================================
# Thomas algoritam za rešavanje tridiagonalnog sistema
def thomas(a_sub, b_diag, c_sup, d_rhs):
    n = len(d_rhs)
    cp = np.zeros(n)
    dp = np.zeros(n)
    
    cp[0] = c_sup[0] / b_diag[0]
    dp[0] = d_rhs[0] / b_diag[0]
    
    for k in range(1, n):
        denom = b_diag[k] - a_sub[k] * cp[k - 1]
        cp[k] = (c_sup[k] / denom) if k < n - 1 else 0.0
        dp[k] = (d_rhs[k] - a_sub[k] * dp[k - 1]) / denom
    
    x_sol = np.zeros(n)
    x_sol[-1] = dp[-1]
    for k in range(n - 2, -1, -1):
        x_sol[k] = dp[k] - cp[k] * x_sol[k + 1]
    return x_sol


print("Računam Crank-Nicolson metodu...")
alpha = C / 4.0

for j in range(Nt - 1):
    left_bc = U_cn[0, j + 1]  # leva granica (vec postavljena)
    
    # Broj nepoznatih: U_cn[1..Nx-1, j+1]
    m = Nx - 1
    
    A = np.zeros(m)     # poddijagonala
    B = np.zeros(m)     # dijagonala
    Cc = np.zeros(m)    # naddijagonala
    D = np.zeros(m)     # desna strana
    
    # Jednačine za i = 1..Nx-2
    for i in range(1, Nx - 1):
        k = i - 1
        A[k] = -alpha
        B[k] = 1.0
        Cc[k] = alpha
        D[k] = U_cn[i, j] - alpha * (U_cn[i + 1, j] - U_cn[i - 1, j])
    
    # Uključivanje leve granice u jednačinu za i=1 (k=0)
    D[0] -= A[0] * left_bc
    A[0] = 0.0
    
    # Poslednja jednačina: outflow uslov U_{Nx-1}^{n+1} - U_{Nx-2}^{n+1} = 0
    A[m - 1] = -1.0
    B[m - 1] = 1.0
    Cc[m - 1] = 0.0
    D[m - 1] = 0.0
    
    # Rešavanje sistema
    sol = thomas(A, B, Cc, D)
    U_cn[1:, j + 1] = sol

print("Rešavanje završeno!")
print()


# =========================================================
# GRAFICI
# =========================================================
# Pojedinačni grafici za svaku metodu
plt.figure(figsize=(10, 6))
plt.plot(x, U_lw[:, -1], 'b-', linewidth=2)
plt.plot(x, U0, 'k--', linewidth=1, alpha=0.5, label='Početni uslov (t=0)')
plt.grid(True, alpha=0.3)
plt.xlabel('x', fontsize=12)
plt.ylabel('U(x, 0.3)', fontsize=12)
plt.title('Lax-Wendroff metoda (t = 0.3s)', fontsize=14, fontweight='bold')
plt.legend()
plt.tight_layout()

plt.figure(figsize=(10, 6))
plt.plot(x, U_mc[:, -1], 'g-', linewidth=2)
plt.plot(x, U0, 'k--', linewidth=1, alpha=0.5, label='Početni uslov (t=0)')
plt.grid(True, alpha=0.3)
plt.xlabel('x', fontsize=12)
plt.ylabel('U(x, 0.3)', fontsize=12)
plt.title('MacCormack metoda (t = 0.3s)', fontsize=14, fontweight='bold')
plt.legend()
plt.tight_layout()

plt.figure(figsize=(10, 6))
plt.plot(x, U_cn[:, -1], 'r-', linewidth=2)
plt.plot(x, U0, 'k--', linewidth=1, alpha=0.5, label='Početni uslov (t=0)')
plt.grid(True, alpha=0.3)
plt.xlabel('x', fontsize=12)
plt.ylabel('U(x, 0.3)', fontsize=12)
plt.title('Crank-Nicolson metoda (t = 0.3s)', fontsize=14, fontweight='bold')
plt.legend()
plt.tight_layout()

# Poređenje svih metoda na jednom grafiku
plt.figure(figsize=(12, 7))
plt.plot(x, U_lw[:, -1], 'b-', linewidth=2.5, label='Lax-Wendroff')
plt.plot(x, U_mc[:, -1], 'g--', linewidth=2.5, label='MacCormack')
plt.plot(x, U_cn[:, -1], 'r:', linewidth=2.5, label='Crank-Nicolson')
plt.plot(x, U0, 'k--', linewidth=1.5, alpha=0.5, label='Početni uslov (t=0)')
plt.grid(True, alpha=0.3)
plt.xlabel('x', fontsize=12)
plt.ylabel('U(x, 0.3)', fontsize=12)
plt.title('Poređenje metoda (t = 0.3s)', fontsize=14, fontweight='bold')
plt.legend(fontsize=11)
plt.tight_layout()

plt.show()

# Ispis vrednosti u nekoliko karakterističnih tačaka
print("\nVrednosti rešenja u t = 0.3s na nekoliko tačaka:")
print("=" * 60)
print(f"{'x':<10} {'Lax-Wendroff':<15} {'MacCormack':<15} {'Crank-Nicolson':<15}")
print("-" * 60)
for i in [0, Nx//4, Nx//2, 3*Nx//4, Nx-1]:
    print(f"{x[i]:<10.2f} {U_lw[i, -1]:<15.6f} {U_mc[i, -1]:<15.6f} {U_cn[i, -1]:<15.6f}")
print("=" * 60)