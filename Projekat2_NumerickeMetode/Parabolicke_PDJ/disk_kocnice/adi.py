import numpy as np
import json

class DiskBrakeSimulatorADI:

    def __init__(self):
        # Fizički parametri
        self.m = 100.0           # kg - masa vozila
        self.L = 0.02            # m - debljina diska (2 cm)
        self.mr = 45.4           # kg - redukovana masa
        self.lambda_val = 0.2    # W/mK - toplotna provodljivost
        self.a = 12.5e-6         # m²/s - koeficijent temperaturne provodljivosti
        self.V0 = 25.0           # m/s - početna brzina
        self.u = 3.0             # m/s² - usporenje
        self.alpha = 40.0        # W/m²K - koeficijent konvekcije
        self.T0 = 25.0           # °C - početna temperatura
        self.Tv = 25.0           # °C - temperatura okoline
        self.dx = 0.002          # m - prostorni korak (0.2 cm)

        self.N = int(self.L / self.dx) + 1
        self.dt = (self.dx ** 2) / (2 * self.a)

        self.dt_half = self.dt / 2
        self.r_half = self.a * self.dt_half / (self.dx ** 2)

        print(f"Inicijalizacija ADI metode:")
        print(f"  Broj tačaka: N = {self.N}")
        print(f"  Prostorni korak: Δx = {self.dx*100:.2f} cm")
        print(f"  Puni vremenski korak: Δt = {self.dt*1000:.4f} ms")
        print(f"  Polu-korak: Δt/2 = {self.dt_half*1000:.4f} ms")
        print(f"  Parametar r' = {self.r_half:.4f}")

    def solve_tridiagonal(self, a, b, c, d):
        n = len(d)
        c_prime = np.zeros(n)
        d_prime = np.zeros(n)
        x = np.zeros(n)

        c_prime[0] = c[0] / b[0]
        d_prime[0] = d[0] / b[0]

        for i in range(1, n):
            m = b[i] - a[i] * c_prime[i-1]
            c_prime[i] = c[i] / m if i < n-1 else 0
            d_prime[i] = (d[i] - a[i] * d_prime[i-1]) / m

        x[n-1] = d_prime[n-1]
        for i in range(n-2, -1, -1):
            x[i] = d_prime[i] - c_prime[i] * x[i+1]

        return x

    def build_system_braking(self, T_old, V):
        N = self.N
        r = self.r_half

        a = np.zeros(N)
        b = np.zeros(N)
        c = np.zeros(N)
        d = np.zeros(N)

        for i in range(1, N-1):
            a[i] = -r
            b[i] = 1 + 2*r
            c[i] = -r
            d[i] = T_old[i]

        Q = self.mr * self.u * V / (2 * self.lambda_val)

        a[0] = 0
        b[0] = 1 + 2*r
        c[0] = -2*r
        d[0] = T_old[0] + 2*r*self.dx*Q

        Bi = self.alpha * self.dx / self.lambda_val

        a[N-1] = -2*r
        b[N-1] = 1 + 2*r*(1 + Bi)
        c[N-1] = 0
        d[N-1] = T_old[N-1] + 2*r*Bi*self.Tv

        return a, b, c, d

    def build_system_cooling(self, T_old):
        N = self.N
        r = self.r_half

        a = np.zeros(N)
        b = np.zeros(N)
        c = np.zeros(N)
        d = np.zeros(N)

        for i in range(1, N-1):
            a[i] = -r
            b[i] = 1 + 2*r
            c[i] = -r
            d[i] = T_old[i]

        Bi = self.alpha * self.dx / self.lambda_val

        # Leva granica: Konvekcija
        a[0] = 0
        b[0] = 1 + 2*r*(1 + Bi)
        c[0] = -2*r
        d[0] = T_old[0] + 2*r*Bi*self.Tv

        a[N-1] = -2*r
        b[N-1] = 1 + 2*r*(1 + Bi)
        c[N-1] = 0
        d[N-1] = T_old[N-1] + 2*r*Bi*self.Tv

        return a, b, c, d

    def adi_step_braking(self, T, V):
        a1, b1, c1, d1 = self.build_system_braking(T, V)
        T_half = self.solve_tridiagonal(a1, b1, c1, d1)

        a2, b2, c2, d2 = self.build_system_braking(T_half, V)
        T_new = self.solve_tridiagonal(a2, b2, c2, d2)

        return T_new

    def adi_step_cooling(self, T):
        a1, b1, c1, d1 = self.build_system_cooling(T)
        T_half = self.solve_tridiagonal(a1, b1, c1, d1)

        a2, b2, c2, d2 = self.build_system_cooling(T_half)
        T_new = self.solve_tridiagonal(a2, b2, c2, d2)

        return T_new

    def simulate(self, cooling_time=10.0, save_interval=0.1):
        T = np.full(self.N, self.T0)
        V = self.V0
        t = 0.0
        braking_distance = 0.0

        time_steps = []
        temp_data = []
        save_counter = 0

        print("\n=== FAZA KOČENJA (ADI) ===")

        while V > 0:
            T = self.adi_step_braking(T, V)

            V -= self.u * self.dt
            if V < 0:
                V = 0

            braking_distance += V * self.dt

            t += self.dt

            if t >= save_counter * save_interval:
                time_steps.append(t)
                temp_data.append(T.copy())
                save_counter += 1

                if save_counter % 10 == 0:
                    print(f"  t = {t:.2f}s, V = {V:.2f}m/s, T_max = {np.max(T):.2f}°C")

        stop_time = t
        print(f"\nVozilo zaustavljeno!")
        print(f"  Vreme kočenja: {stop_time:.3f} s")
        print(f"  Trag kočenja: {braking_distance:.2f} m")
        print(f"  Maksimalna temperatura: {np.max(T):.2f}°C")

        print("\n=== FAZA HLAĐENJA (ADI) ===")

        end_time = stop_time + cooling_time
        while t < end_time:
            T = self.adi_step_cooling(T)

            t += self.dt

            if t >= save_counter * save_interval:
                time_steps.append(t)
                temp_data.append(T.copy())
                save_counter += 1

                if (save_counter - int(stop_time/save_interval)) % 10 == 0:
                    print(f"  t = {t:.2f}s, T_max = {np.max(T):.2f}°C")

        print(f"  Finalna maksimalna temperatura: {np.max(T):.2f}°C")
        results = {
            'method': 'ADI',
            'time_steps': time_steps,
            'temp_data': [T_i.tolist() for T_i in temp_data],
            'stop_time': stop_time,
            'braking_distance': braking_distance,
            'final_temp': T.tolist(),
            'x_coords': [i * self.dx for i in range(self.N)],
            'parameters': {
                'N': self.N,
                'dx': self.dx,
                'dt': self.dt,
                'dt_half': self.dt_half,
                'L': self.L,
                'a': self.a,
                'V0': self.V0,
                'u': self.u
            }
        }

        return results


def main():
    print("="*60)
    print("DISK KOČNICE - ADI METODA")
    print("="*60)

    # Kreiranje simulatora
    sim = DiskBrakeSimulatorADI()

    # Pokretanje simulacije
    results = sim.simulate(cooling_time=10.0, save_interval=0.1)

    # Čuvanje rezultata
    with open('results_adi.json', 'w') as f:
        json.dump(results, f, indent=2)

    print("\n" + "="*60)
    print("Rezultati sačuvani u: results_adi.json")
    print("="*60)


if __name__ == "__main__":
    main()
