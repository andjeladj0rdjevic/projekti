import numpy as np
import json


class DiskBrakeSimulator:

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

        # Proračunski parametri
        self.N = int(self.L / self.dx) + 1
        self.dt = (self.dx ** 2) / (2 * self.a)
        self.r = self.a * self.dt / (self.dx ** 2)

        print(f"Inicijalizacija Laasonen metode:")
        print(f"  Broj tačaka: N = {self.N}")
        print(f"  Prostorni korak: Δx = {self.dx*100:.2f} cm")
        print(f"  Vremenski korak: Δt = {self.dt*1000:.4f} ms")
        print(f"  Parametar r = {self.r:.4f}")

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
        """
        Leva granica (x=0): Toplotni fluks od trenja
          Granični uslov iz zadatka: ∂T/∂x = (u*mr)/(2λ) * (2V_i - u*Δτ)
          Fantom tačka: T_{-1} = T_1 + 2*Δx * (∂T/∂x)|_0
          Rezultat: (1+2r)*T_0 - 2r*T_1 = T_0^n + 2r*Δx * (∂T/∂x)|_0

        Desna granica (x=L): Konvekcija
          Granični uslov: -λ ∂T/∂x = α*(T_p - Tv)
          Biot-ov broj: Bi = α*Δx/λ
          Rezultat: -2r*T_{N-2} + (1+2r*(1+Bi))*T_{N-1} = T_{N-1}^n + 2r*Bi*Tv
        """
        N = self.N
        r = self.r

        a = np.zeros(N)
        b = np.zeros(N)
        c = np.zeros(N)
        d = np.zeros(N)

        # Unutrašnje tačke (i = 1, ..., N-2)
        for i in range(1, N-1):
            a[i] = -r
            b[i] = 1 + 2*r
            c[i] = -r
            d[i] = T_old[i]

        # Leva granica (i = 0): Toplotni fluks
        # Iz zadatka: ∂T/∂x|_0 = (u*mr)/(2λ) * (2V_i - u*Δτ)
        grad_T = self.u * self.mr / (2 * self.lambda_val) * (2 * V - self.u * self.dt)

        a[0] = 0
        b[0] = 1 + 2*r
        c[0] = -2*r
        d[0] = T_old[0] + 2*r*self.dx * grad_T

        # Desna granica (i = N-1): Konvekcija
        Bi = self.alpha * self.dx / self.lambda_val

        a[N-1] = -2*r
        b[N-1] = 1 + 2*r*(1 + Bi)
        c[N-1] = 0
        d[N-1] = T_old[N-1] + 2*r*Bi*self.Tv

        return a, b, c, d

    def build_system_cooling(self, T_old):
        """
        Obe granice: Konvekcija (nakon zaustavljanja)
        """
        N = self.N
        r = self.r

        a = np.zeros(N)
        b = np.zeros(N)
        c = np.zeros(N)
        d = np.zeros(N)

        # Unutrašnje tačke
        for i in range(1, N-1):
            a[i] = -r
            b[i] = 1 + 2*r
            c[i] = -r
            d[i] = T_old[i]

        Bi = self.alpha * self.dx / self.lambda_val

        # Leva granica: konvekcija
        a[0] = 0
        b[0] = 1 + 2*r*(1 + Bi)
        c[0] = -2*r
        d[0] = T_old[0] + 2*r*Bi*self.Tv

        # Desna granica: konvekcija
        a[N-1] = -2*r
        b[N-1] = 1 + 2*r*(1 + Bi)
        c[N-1] = 0
        d[N-1] = T_old[N-1] + 2*r*Bi*self.Tv

        return a, b, c, d

    def simulate(self, cooling_time=10.0, save_interval=0.1):
        T = np.full(self.N, self.T0)
        V = self.V0
        t = 0.0
        braking_distance = 0.0

        time_steps = []
        temp_data = []
        save_counter = 0

        print("\n=== FAZA KOČENJA (Laasonen) ===")

        while V > 0:
            T_old = T.copy()

            # Rešavanje sistema sa trenutnom brzinom V
            a, b, c, d = self.build_system_braking(T_old, V)
            T = self.solve_tridiagonal(a, b, c, d)

            # Trag kočenja: koristimo V pre ažuriranja (trapezna formula)
            V_old = V
            V -= self.u * self.dt
            if V < 0:
                V = 0

            # Pređeni put u ovom koraku (prosek stare i nove brzine)
            braking_distance += 0.5 * (V_old + V) * self.dt

            t += self.dt

            if t >= save_counter * save_interval:
                time_steps.append(round(t, 6))
                temp_data.append(T.copy())
                save_counter += 1

                if save_counter % 10 == 0:
                    print(f"  t = {t:.2f}s, V = {V:.2f}m/s, T_max = {np.max(T):.2f}°C")

        stop_time = t
        print(f"\nVozilo zaustavljeno!")
        print(f"  Vreme kočenja: {stop_time:.3f} s")
        print(f"  Trag kočenja: {braking_distance:.2f} m")
        print(f"  Maksimalna temperatura: {np.max(T):.2f}°C")

        print("\n=== FAZA HLAĐENJA (Laasonen) ===")

        end_time = stop_time + cooling_time
        while t < end_time:
            T_old = T.copy()

            a, b, c, d = self.build_system_cooling(T_old)
            T = self.solve_tridiagonal(a, b, c, d)

            t += self.dt

            if t >= save_counter * save_interval:
                time_steps.append(round(t, 6))
                temp_data.append(T.copy())
                save_counter += 1

                if (save_counter - int(stop_time / save_interval)) % 10 == 0:
                    print(f"  t = {t:.2f}s, T_max = {np.max(T):.2f}°C")

        print(f"  Finalna maksimalna temperatura: {np.max(T):.2f}°C")

        results = {
            'method': 'Laasonen',
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
                'L': self.L,
                'a': self.a,
                'V0': self.V0,
                'u': self.u,
                'T0': self.T0
            }
        }

        return results


def main():
    print("=" * 60)
    print("DISK KOČNICE - LAASONEN METODA")
    print("=" * 60)

    sim = DiskBrakeSimulator()
    results = sim.simulate(cooling_time=10.0, save_interval=0.1)

    with open('results_laasonen.json', 'w') as f:
        json.dump(results, f, indent=2)

    print("\n" + "=" * 60)
    print("Rezultati sačuvani u: results_laasonen.json")
    print("=" * 60)


if __name__ == "__main__":
    main()
