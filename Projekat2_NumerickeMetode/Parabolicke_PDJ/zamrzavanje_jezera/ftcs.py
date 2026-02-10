import numpy as np
import matplotlib.pyplot as plt

def solve_stefan_problem_fast():
   
    lambda_L = 1.6      
    alpha_L = 0.863e-6  
    rho = 917.0         
    L_fusion = 334000.0 
    
    T_m = 0.0           
    T_air = -10.0       
    
    t_max = 24 * 3600   # 24 sata u sekundama
    
    N = 10              
    d_xi = 1.0 / (N - 1)
    
    d_current = 0.005   # Pocinjemo sa 5mm leda radi stabilnosti
    t = 0
    
    time_history = []
    thickness_history = []
    
    xi = np.linspace(0, 1, N)
    T_L = T_air + (T_m - T_air) * xi 

    print("Simulacija pokrenuta (optimizovana verzija)...")


    while t < t_max:
        
        grad_T_L_x = (T_L[-1] - T_L[-2]) / (d_xi * d_current)
        
        v = (lambda_L * grad_T_L_x) / (rho * L_fusion)
        
        dx = d_current * d_xi
        dt = 0.4 * (dx**2) / alpha_L 

        dt = min(dt, 30.0) 

        d_current += v * dt
  
        T_L_new = np.copy(T_L)
        r = (alpha_L * dt) / (dx**2)
        c = (v * dt) / (d_current * d_xi)
        
        for i in range(1, N-1):
            T_L_new[i] = T_L[i] + r*(T_L[i+1] - 2*T_L[i] + T_L[i-1]) + xi[i]*c*(T_L[i+1]-T_L[i])
            
        T_L = T_L_new
        t += dt

        if len(time_history) == 0 or (t - time_history[-1]*3600) > 900:
            time_history.append(t / 3600.0)
            thickness_history.append(d_current * 1000.0)

    print(f"Simulacija zavrsena! Konacna debljina: {d_current*1000:.2f} mm")

    plt.figure(figsize=(10, 4))
    plt.subplot(1, 2, 1)
    plt.plot(time_history, thickness_history, 'b', linewidth=2)
    plt.xlabel('Vreme [h]'); plt.ylabel('Debljina leda [mm]'); plt.grid(True)
    
    plt.subplot(1, 2, 2)
    plt.plot(xi * d_current * 1000, T_L, 'r-o')
    plt.xlabel('Dubina [mm]'); plt.ylabel('Temperatura [C]'); plt.grid(True)
    plt.tight_layout()
    plt.show()

solve_stefan_problem_fast()
