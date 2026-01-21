import json
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator

def load_results(filename):
    with open(filename, 'r') as f:
        return json.load(f)

def plot_temperature_evolution(results, title_suffix=""):
    time_steps = results['time_steps']
    temp_data = results['temp_data']
    x_coords = results['x_coords']
    stop_time = results['stop_time']

    # Konverzija u cm
    x_cm = [x * 100 for x in x_coords]

    # Odabir vremenskih trenutaka za prikaz
    n_plots = 8
    indices = np.linspace(0, len(time_steps)-1, n_plots, dtype=int)

    plt.figure(figsize=(12, 8))

    colors = plt.cm.jet(np.linspace(0, 1, n_plots))

    for i, idx in enumerate(indices):
        t = time_steps[idx]
        T = temp_data[idx]

        # Oznaka faze
        if t <= stop_time:
            label = f't = {t:.2f}s (kočenje)'
        else:
            label = f't = {t:.2f}s (hlađenje)'

        plt.plot(x_cm, T, 'o-', color=colors[i], linewidth=2,
                markersize=6, label=label)

    plt.xlabel('Pozicija kroz disk [cm]', fontsize=12)
    plt.ylabel('Temperatura [°C]', fontsize=12)
    plt.title(f'Evolucija Temperature - {title_suffix}', fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.legend(loc='best', fontsize=10)
    plt.tight_layout()

    return plt.gcf()

def plot_final_temperature(results, title_suffix=""):
    x_coords = results['x_coords']
    final_temp = results['final_temp']

    x_cm = [x * 100 for x in x_coords]

    plt.figure(figsize=(10, 6))

    plt.plot(x_cm, final_temp, 'o-', color='purple',
            linewidth=3, markersize=8, label='Finalna temperatura')

    plt.axhline(y=results['parameters'].get('T0', 25),
               color='green', linestyle='--', linewidth=2,
               label='Početna temperatura')

    plt.xlabel('Pozicija kroz disk [cm]', fontsize=12)
    plt.ylabel('Temperatura [°C]', fontsize=12)
    plt.title(f'Raspodela Temperature 10s nakon Zaustavljanja - {title_suffix}',
             fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.legend(fontsize=11)
    plt.tight_layout()

    return plt.gcf()

def plot_max_temperature_vs_time(results, title_suffix=""):
    time_steps = results['time_steps']
    temp_data = results['temp_data']
    stop_time = results['stop_time']

    max_temps = [np.max(T) for T in temp_data]

    plt.figure(figsize=(12, 6))

    # Podela na faze
    braking_mask = np.array(time_steps) <= stop_time
    cooling_mask = np.array(time_steps) > stop_time

    t_brake = np.array(time_steps)[braking_mask]
    T_brake = np.array(max_temps)[braking_mask]

    t_cool = np.array(time_steps)[cooling_mask]
    T_cool = np.array(max_temps)[cooling_mask]

    plt.plot(t_brake, T_brake, 'r-', linewidth=2.5, label='Faza kočenja')
    plt.plot(t_cool, T_cool, 'b-', linewidth=2.5, label='Faza hlađenja')

    plt.axvline(x=stop_time, color='black', linestyle='--',
               linewidth=2, label=f'Zaustavljanje (t={stop_time:.2f}s)')

    plt.xlabel('Vreme [s]', fontsize=12)
    plt.ylabel('Maksimalna Temperatura [°C]', fontsize=12)
    plt.title(f'Maksimalna Temperatura u Funkciji Vremena - {title_suffix}',
             fontsize=14, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.legend(fontsize=11)
    plt.tight_layout()

    return plt.gcf()

def plot_3d_surface(results, title_suffix=""):
    time_steps = results['time_steps']
    temp_data = results['temp_data']
    x_coords = results['x_coords']

    # Kreiranje mreže
    X = np.array(x_coords) * 100  # cm
    T = np.array(time_steps)
    X, T = np.meshgrid(X, T)
    Z = np.array(temp_data)

    fig = plt.figure(figsize=(14, 10))
    ax = fig.add_subplot(111, projection='3d')

    surf = ax.plot_surface(X, T, Z, cmap=cm.jet,
                          linewidth=0, antialiased=True)

    ax.set_xlabel('Pozicija [cm]', fontsize=11)
    ax.set_ylabel('Vreme [s]', fontsize=11)
    ax.set_zlabel('Temperatura [°C]', fontsize=11)
    ax.set_title(f'3D Prikaz: T(x, t) - {title_suffix}',
                fontsize=14, fontweight='bold')

    fig.colorbar(surf, shrink=0.5, aspect=5, label='Temperatura [°C]')

    plt.tight_layout()

    return fig

def plot_contour(results, title_suffix=""):
    time_steps = results['time_steps']
    temp_data = results['temp_data']
    x_coords = results['x_coords']
    stop_time = results['stop_time']

    X = np.array(x_coords) * 100  # cm
    T = np.array(time_steps)
    Z = np.array(temp_data)

    plt.figure(figsize=(12, 8))

    contour = plt.contourf(X, T, Z, levels=20, cmap='jet')
    plt.colorbar(contour, label='Temperatura [°C]')

    plt.axhline(y=stop_time, color='white', linestyle='--',
               linewidth=2, label=f'Zaustavljanje (t={stop_time:.2f}s)')

    plt.xlabel('Pozicija kroz disk [cm]', fontsize=12)
    plt.ylabel('Vreme [s]', fontsize=12)
    plt.title(f'Contour Plot: Temperatura T(x, t) - {title_suffix}',
             fontsize=14, fontweight='bold')
    plt.legend(fontsize=11)
    plt.tight_layout()

    return plt.gcf()

def compare_methods(results_laasonen, results_adi):
    x_coords = results_laasonen['x_coords']
    x_cm = [x * 100 for x in x_coords]

    fig, axes = plt.subplots(2, 2, figsize=(14, 10))

    ax1 = axes[0, 0]
    ax1.plot(x_cm, results_laasonen['final_temp'], 'o-',
            linewidth=2, markersize=6, label='Laasonen')
    ax1.plot(x_cm, results_adi['final_temp'], 's--',
            linewidth=2, markersize=6, label='ADI')
    ax1.set_xlabel('Pozicija [cm]')
    ax1.set_ylabel('Temperatura [°C]')
    ax1.set_title('Finalna Raspodela Temperature')
    ax1.grid(True, alpha=0.3)
    ax1.legend()

    ax2 = axes[0, 1]
    diff = np.array(results_laasonen['final_temp']) - np.array(results_adi['final_temp'])
    ax2.plot(x_cm, diff, 'o-', color='red', linewidth=2, markersize=6)
    ax2.axhline(y=0, color='black', linestyle='--', linewidth=1)
    ax2.set_xlabel('Pozicija [cm]')
    ax2.set_ylabel('Razlika Temperature [°C]')
    ax2.set_title('Laasonen - ADI')
    ax2.grid(True, alpha=0.3)

    ax3 = axes[1, 0]
    max_temps_l = [np.max(T) for T in results_laasonen['temp_data']]
    max_temps_a = [np.max(T) for T in results_adi['temp_data']]
    ax3.plot(results_laasonen['time_steps'], max_temps_l,
            linewidth=2, label='Laasonen')
    ax3.plot(results_adi['time_steps'], max_temps_a,
            '--', linewidth=2, label='ADI')
    ax3.set_xlabel('Vreme [s]')
    ax3.set_ylabel('Maksimalna Temperatura [°C]')
    ax3.set_title('Maksimalna Temperatura vs Vreme')
    ax3.grid(True, alpha=0.3)
    ax3.legend()

    ax4 = axes[1, 1]
    ax4.axis('off')

    comparison_text = f"""
    POREĐENJE METODA

    LAASONEN:
    • Trag kočenja: {results_laasonen['braking_distance']:.2f} m
    • Vreme zaustavljanja: {results_laasonen['stop_time']:.3f} s
    • Max T finalna: {np.max(results_laasonen['final_temp']):.2f} °C

    ADI:
    • Trag kočenja: {results_adi['braking_distance']:.2f} m
    • Vreme zaustavljanja: {results_adi['stop_time']:.3f} s
    • Max T finalna: {np.max(results_adi['final_temp']):.2f} °C

    RAZLIKE:
    • Trag: {abs(results_laasonen['braking_distance'] - results_adi['braking_distance']):.4f} m
    • Vreme: {abs(results_laasonen['stop_time'] - results_adi['stop_time']):.6f} s
    • Max T: {abs(np.max(results_laasonen['final_temp']) - np.max(results_adi['final_temp'])):.4f} °C

    Max razlika u T: {np.max(np.abs(diff)):.6f} °C
    """

    ax4.text(0.1, 0.5, comparison_text, fontsize=11,
            verticalalignment='center', fontfamily='monospace')

    plt.tight_layout()

    return fig

def print_summary(results):
    print("\n" + "="*60)
    print(f"SAŽETAK REZULTATA - {results['method']}")
    print("="*60)

    print("\nParametri simulacije:")
    params = results['parameters']
    print(f"  Broj tačaka: N = {params['N']}")
    print(f"  Prostorni korak: Δx = {params['dx']*100:.2f} cm")
    print(f"  Vremenski korak: Δt = {params['dt']*1000:.4f} ms")
    print(f"  Debljina diska: L = {params['L']*100:.1f} cm")

    print("\nRezultati kočenja:")
    print(f"  Trag kočenja: {results['braking_distance']:.2f} m")
    print(f"  Vreme zaustavljanja: {results['stop_time']:.3f} s")
    print(f"  Maksimalna temperatura: {np.max(results['temp_data']):.2f} °C")

    print("\nFinalna stanje (10s posle):")
    print(f"  Maksimalna temperatura: {np.max(results['final_temp']):.2f} °C")
    print(f"  Minimalna temperatura: {np.min(results['final_temp']):.2f} °C")
    print(f"  Prosečna temperatura: {np.mean(results['final_temp']):.2f} °C")

    print("="*60)

def main():
    print("="*60)
    print("POSTPROCESIRANJE - DISK KOČNICE")
    print("="*60)

    # Učitavanje rezultata
    try:
        results_laasonen = load_results('results_laasonen.json')
        print("\n✓ Učitani rezultati: Laasonen metoda")
    except FileNotFoundError:
        print("\n✗ Nije pronađen fajl: results_laasonen.json")
        results_laasonen = None

    try:
        results_adi = load_results('results_adi.json')
        print("✓ Učitani rezultati: ADI metoda")
    except FileNotFoundError:
        print("✗ Nije pronađen fajl: results_adi.json")
        results_adi = None

    if results_laasonen is None and results_adi is None:
        print("\nGreška: Nema rezultata za prikaz!")
        return

    # Kreiranje foldera za slike
    import os
    if not os.path.exists('figures'):
        os.makedirs('figures')
        print("\n✓ Kreiran folder: figures/")

    # SAMO NAJVAŽNIJI GRAFICI

    if results_laasonen:
        print_summary(results_laasonen)

        print("\nKreiranje grafika - Laasonen metoda...")

        # 1. Evolucija temperature (najvažniji)
        fig1 = plot_temperature_evolution(results_laasonen, "Laasonen")
        fig1.savefig('figures/laasonen_evolution.png', dpi=150)

        # 2. Finalna raspodela (ključni rezultat)
        fig2 = plot_final_temperature(results_laasonen, "Laasonen")
        fig2.savefig('figures/laasonen_final.png', dpi=150)

        print("  ✓ Sačuvano 2 grafika")

    if results_adi:
        print_summary(results_adi)

        print("\nKreiranje grafika - ADI metoda...")

        # 1. Evolucija temperature
        fig1 = plot_temperature_evolution(results_adi, "ADI")
        fig1.savefig('figures/adi_evolution.png', dpi=150)

        # 2. Finalna raspodela
        fig2 = plot_final_temperature(results_adi, "ADI")
        fig2.savefig('figures/adi_final.png', dpi=150)

        print("  ✓ Sačuvano 2 grafika")

    # Poređenje metoda (ako oba postoje)
    if results_laasonen and results_adi:
        print("\nKreiranje grafika za poređenje...")

        fig_comp = compare_methods(results_laasonen, results_adi)
        fig_comp.savefig('figures/comparison.png', dpi=150)

        print("  ✓ Sačuvan grafik poređenja")

    print("\n" + "="*60)
    print(f"Ukupno grafika: {2 + 2 + 1} (po 2 za svaku metodu + poređenje)")
    print(f"Grafici sačuvani u folderu: figures/")
    print("="*60)

    plt.show()


if __name__ == "__main__":
    main()
