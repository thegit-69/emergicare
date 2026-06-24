import React, { useState, useEffect, useMemo } from 'react';
import { BrowserRouter as Router, Routes, Route, Link, useLocation, useNavigate, useParams, Navigate } from 'react-router-dom';
import axios from 'axios';

// --- REAL API SETUP ---
const API_URL = import.meta.env.VITE_API_BASE_URL || 'http://localhost:3001/api';

const parseJwt = (token) => {
    try {
        const base64Url = token.split('.')[1];
        const base64 = base64Url.replace(/-/g, '+').replace(/_/g, '/');
        const jsonPayload = decodeURIComponent(
            window.atob(base64)
                .split('')
                .map((c) => '%' + ('00' + c.charCodeAt(0).toString(16)).slice(-2))
                .join('')
        );
        return JSON.parse(jsonPayload);
    } catch (e) {
        return null;
    }
};

const getUtcDate = (dateString) => {
    if (!dateString) return new Date();
    if (dateString.endsWith('Z') || dateString.match(/[\+\-]\d{2}:\d{2}$/)) return new Date(dateString);
    return new Date(dateString.replace(' ', 'T') + 'Z');
};

const initialToken = localStorage.getItem('healthcare_token');
if (initialToken) {
    axios.defaults.headers.common['Authorization'] = `Bearer ${initialToken}`;
}

const api = {
    login: async (username, password) => { const response = await axios.post(`${API_URL}/login`, { username, password }); return response.data; },
    registerUser: async (username, password, role, name) => { const response = await axios.post(`${API_URL}/users`, { username, password, role, name }); return { success: true, user: response.data }; },
    getUsers: async () => { const response = await axios.get(`${API_URL}/users`); return response.data; },
    getPatients: async () => { const response = await axios.get(`${API_URL}/patients`); return response.data; },
    addPatient: async (patient) => { const response = await axios.post(`${API_URL}/patients`, patient); return response.data; },
    updatePatient: async (patient) => { const response = await axios.put(`${API_URL}/patients/${patient.id}`, patient); return response.data; },
    deletePatient: async (patientId) => { const response = await axios.delete(`${API_URL}/patients/${patientId}`); return response.data; },
    getMedicalRecords: async () => { const response = await axios.get(`${API_URL}/medical-records`); return response.data; },
    addMedicalRecord: async (record) => { const response = await axios.post(`${API_URL}/medical-records`, record); return response.data; },
    getAppointments: async () => { const response = await axios.get(`${API_URL}/appointments`); return response.data; },
    addAppointment: async (appointment) => { const response = await axios.post(`${API_URL}/appointments`, appointment); return response.data; },

    // NEW EMERGENCY ENDPOINTS
    getTriageEntries: async () => { const response = await axios.get(`${API_URL}/triage`); return response.data; },
    addTriageEntry: async (entry) => { const response = await axios.post(`${API_URL}/triage`, entry); return response.data; },
    updateTriageStatus: async (id, status) => { const response = await axios.put(`${API_URL}/triage/${id}`, { status }); return response.data; },

    getEmergencyQueue: async () => { const response = await axios.get(`${API_URL}/emergency-queue`); return response.data; },
    enqueuePatient: async (entry) => { const response = await axios.post(`${API_URL}/emergency-queue`, entry); return response.data; },
    updateQueueStatus: async (id, data) => { const response = await axios.put(`${API_URL}/emergency-queue/${id}`, data); return response.data; },
    dequeuePatient: async (id) => { const response = await axios.delete(`${API_URL}/emergency-queue/${id}`); return response.data; },

    getResources: async () => { const response = await axios.get(`${API_URL}/resources`); return response.data; },
    addResource: async (res) => { const response = await axios.post(`${API_URL}/resources`, res); return response.data; },
    updateResource: async (id, data) => { const response = await axios.put(`${API_URL}/resources/${id}`, data); return response.data; },
};

// --- UI COMPONENTS ---
const Card = ({ title, children, className, actions }) => (
    <div className={`bg-white rounded-xl shadow-md ${className || ''}`}>
        {title && <div className="p-4 md:p-6 border-b border-gray-200 flex justify-between items-center"><h2 className="text-lg md:text-xl font-bold text-gray-800">{title}</h2><div>{actions}</div></div>}
        <div className="p-4 md:p-6">{children}</div>
    </div>
);
const StatCard = ({ icon, label, value, color }) => (
    <div className="bg-white rounded-xl shadow-md p-4 flex items-center">
        <div className={`rounded-full p-3 mr-4 ${color}`}>{icon}</div>
        <div>
            <p className="text-sm text-gray-500">{label}</p>
            <p className="text-2xl font-bold text-gray-800">{value}</p>
        </div>
    </div>
);

const SubmitButton = ({ isSubmitting, text, loadingText, className }) => (
    <button type="submit" disabled={isSubmitting} className={`${className} ${isSubmitting ? 'opacity-70 cursor-not-allowed' : ''}`}>
        {isSubmitting ? loadingText : text}
    </button>
);

const LoadingSpinner = ({ fullScreen }) => (
    <div className={`flex justify-center items-center ${fullScreen ? 'min-h-screen' : 'p-4'}`}>
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-red-600"></div>
    </div>
);


// --- VIEWS ---
const AuthScreen = () => {
    const [user, setUser] = useState(() => {
        const token = localStorage.getItem('healthcare_token');
        if (token) {
            const decoded = parseJwt(token);
            if (decoded && decoded.exp * 1000 > Date.now()) return decoded;
            localStorage.removeItem('healthcare_token');
        }
        return null;
    });

    const LoginComponent = () => {
        const [username, setUsername] = useState('');
        const [password, setPassword] = useState('');
        const [error, setError] = useState('');
        const [isSubmitting, setIsSubmitting] = useState(false);

        const handleLogin = async (e) => {
            e.preventDefault();
            setIsSubmitting(true);
            try {
                const result = await api.login(username, password);
                if (result.success) {
                    localStorage.setItem('healthcare_token', result.token);
                    axios.defaults.headers.common['Authorization'] = `Bearer ${result.token}`;
                    setUser(result.user);
                }
            } catch (err) {
                setError("Invalid credentials or server error.");
            } finally {
                setIsSubmitting(false);
            }
        };

        return (
            <div className="min-h-screen bg-gray-50 flex items-center justify-center p-4">
                <div className="max-w-md w-full bg-white p-8 rounded-xl shadow-lg border-t-4 border-red-600">
                    <div className="text-center mb-8"><h1 className="text-3xl font-extrabold text-red-600 tracking-tight">EmergiCare</h1><h3 className="text-md font-medium text-gray-500 mt-2">Adaptive Emergency Coordination</h3></div>
                    {error && <p className="bg-red-100 text-red-700 p-3 rounded-md mb-4 text-sm font-medium">{error}</p>}
                    <form onSubmit={handleLogin} className="space-y-4">
                        <input type="text" placeholder="Username" value={username} onChange={(e) => setUsername(e.target.value)} className="w-full p-3 border border-gray-300 rounded-md focus:ring-2 focus:ring-red-500 outline-none" required />
                        <input type="password" placeholder="Password" value={password} onChange={(e) => setPassword(e.target.value)} className="w-full p-3 border border-gray-300 rounded-md focus:ring-2 focus:ring-red-500 outline-none" required />
                        <SubmitButton isSubmitting={isSubmitting} text="Login to Dashboard" loadingText="Authenticating..." className="w-full py-3 px-4 bg-red-600 text-white rounded-lg hover:bg-red-700 transition duration-300 font-semibold shadow-md" />
                    </form>
                </div>
            </div>
        );
    };

    if (user) return <AppLayout user={user} onLogout={() => { localStorage.removeItem('healthcare_token'); setUser(null); }} />;
    return <LoginComponent />;
};

const DashboardView = ({ stats }) => (
    <div className="space-y-6">
        <h1 className="text-2xl font-bold text-gray-800">Hospital Overview</h1>
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-6">
            <StatCard label="Patients in Queue" value={stats.activeQueueCount} color="bg-red-100" icon={<svg className="h-6 w-6 text-red-600" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>} />
            <StatCard label="Critical (P1) Cases" value={stats.criticalCount} color="bg-orange-100" icon={<svg className="h-6 w-6 text-orange-600" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13 10V3L4 14h7v7l9-11h-7z" /></svg>} />
            <StatCard label="Total Patients" value={stats.totalPatients} color="bg-blue-100" icon={<svg className="h-6 w-6 text-blue-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0z" /></svg>} />
            <StatCard label="Resources In Use" value={stats.resourcesInUse} color="bg-indigo-100" icon={<svg className="h-6 w-6 text-indigo-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 21V5a2 2 0 00-2-2H7a2 2 0 00-2 2v16m14 0h2m-2 0h-5m-9 0H3m2 0h5M9 7h1m-1 4h1m4-4h1m-1 4h1m-5 10v-5a1 1 0 011-1h2a1 1 0 011 1v5m-4 0h4" /></svg>} />
        </div>
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
            <Card title="Active Emergency Queue">
                <div className="overflow-x-auto">
                    <table className="w-full text-left min-w-[400px]">
                        <thead>
                            <tr> <th className="p-3 border-b">Patient</th> <th className="p-3 border-b">Priority</th> <th className="p-3 border-b">Status</th> </tr>
                        </thead>
                        <tbody>
                            {stats.emergencyQueue.slice(0, 5).map(q => (
                                <tr key={q.id} className="border-b hover:bg-gray-50">
                                    <td className="p-3 font-medium">{q.patientName}</td>
                                    <td className="p-3"><span className={`px-2 py-1 text-xs font-bold rounded-full ${q.priorityScore === 1 ? 'bg-red-100 text-red-800' : q.priorityScore === 2 ? 'bg-orange-100 text-orange-800' : 'bg-yellow-100 text-yellow-800'}`}>P{q.priorityScore}</span></td>
                                    <td className="p-3 text-sm text-gray-600">{q.queueStatus}</td>
                                </tr>
                            ))}
                            {stats.emergencyQueue.length === 0 && <tr><td colSpan="3" className="p-3 text-gray-500 text-center">Queue is empty</td></tr>}
                        </tbody>
                    </table>
                </div>
            </Card>
            <Card title="Upcoming Standard Appointments">
                <div className="overflow-x-auto">
                    <table className="w-full text-left min-w-[400px]">
                        <thead> <tr> <th className="p-3 border-b">Patient</th> <th className="p-3 border-b">Time</th> </tr> </thead>
                        <tbody>
                            {stats.appointments.slice(0, 5).map(app => (
                                <tr key={app.id} className="border-b hover:bg-gray-50">
                                    <td className="p-3 font-medium">{app.patientName}</td>
                                    <td className="p-3 text-gray-600">{getUtcDate(app.date).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}</td>
                                </tr>
                            ))}
                            {stats.appointments.length === 0 && <tr><td colSpan="2" className="p-3 text-gray-500 text-center">No appointments today</td></tr>}
                        </tbody>
                    </table>
                </div>
            </Card>
        </div>
    </div>
);

// --- NEW VIEWS FOR EMERGICARE ---

const TriageView = ({ triageEntries, setTriageEntries, patients, user }) => {
    const [isAssessing, setIsAssessing] = useState(false);
    const [entry, setEntry] = useState({ patientId: '', chiefComplaint: '', heartRate: '', systolicBp: '', diastolicBp: '', temperature: '', oxygenSat: '', severityOverride: '', notes: '' });

    const handleSubmit = async (e) => {
        e.preventDefault();
        const payload = { ...entry };
        Object.keys(payload).forEach(k => { if (payload[k] === '') delete payload[k]; });
        payload.patientId = parseInt(payload.patientId);
        if (payload.heartRate) payload.heartRate = parseInt(payload.heartRate);
        if (payload.systolicBp) payload.systolicBp = parseInt(payload.systolicBp);
        if (payload.diastolicBp) payload.diastolicBp = parseInt(payload.diastolicBp);
        if (payload.temperature) payload.temperature = parseFloat(payload.temperature);
        if (payload.oxygenSat) payload.oxygenSat = parseInt(payload.oxygenSat);

        try {
            const res = await api.addTriageEntry(payload);
            setTriageEntries([res, ...triageEntries]);
            setIsAssessing(false);
            setEntry({ patientId: '', chiefComplaint: '', heartRate: '', systolicBp: '', diastolicBp: '', temperature: '', oxygenSat: '', severityOverride: '', notes: '' });
        } catch (err) { alert("Failed to assess patient."); }
    };

    return (
        <div className="space-y-6">
            <div className="flex justify-between items-center">
                <h1 className="text-2xl font-bold text-gray-800">Triage System</h1>
                {['Admin', 'Doctor', 'Nurse'].includes(user.role) && <button onClick={() => setIsAssessing(!isAssessing)} className="px-4 py-2 bg-red-600 text-white rounded-lg hover:bg-red-700">{isAssessing ? 'Cancel' : '+ New Assessment'}</button>}
            </div>
            {isAssessing && (
                <Card title="Patient Assessment (Function Overloading Demo)">
                    <form onSubmit={handleSubmit} className="grid grid-cols-1 md:grid-cols-3 gap-4">
                        <select value={entry.patientId} onChange={(e) => setEntry({ ...entry, patientId: e.target.value })} className="p-2 border rounded-md" required>
                            <option value="">Select Patient...</option>
                            {patients.map(p => <option key={p.id} value={p.id}>{p.name}</option>)}
                        </select>
                        <input type="text" placeholder="Chief Complaint (e.g. Chest pain)" value={entry.chiefComplaint} onChange={(e) => setEntry({ ...entry, chiefComplaint: e.target.value })} className="p-2 border rounded-md md:col-span-2" />

                        <div className="md:col-span-3 mt-2 font-semibold text-gray-700 border-b pb-2">Vital Signs (Optional)</div>
                        <input type="number" placeholder="Heart Rate (bpm)" value={entry.heartRate} onChange={(e) => setEntry({ ...entry, heartRate: e.target.value })} className="p-2 border rounded-md" />
                        <input type="number" placeholder="Systolic BP" value={entry.systolicBp} onChange={(e) => setEntry({ ...entry, systolicBp: e.target.value })} className="p-2 border rounded-md" />
                        <input type="number" placeholder="Diastolic BP" value={entry.diastolicBp} onChange={(e) => setEntry({ ...entry, diastolicBp: e.target.value })} className="p-2 border rounded-md" />
                        <input type="number" step="0.1" placeholder="Temp (°C)" value={entry.temperature} onChange={(e) => setEntry({ ...entry, temperature: e.target.value })} className="p-2 border rounded-md" />
                        <input type="number" placeholder="O2 Sat (%)" value={entry.oxygenSat} onChange={(e) => setEntry({ ...entry, oxygenSat: e.target.value })} className="p-2 border rounded-md" />
                        <select value={entry.severityOverride} onChange={(e) => setEntry({ ...entry, severityOverride: e.target.value })} className="p-2 border rounded-md">
                            <option value="">-- No Manual Override --</option>
                            <option value="P1">P1 - Immediate</option>
                            <option value="P2">P2 - Urgent</option>
                            <option value="P3">P3 - Less Urgent</option>
                            <option value="P4">P4 - Non-Urgent</option>
                        </select>

                        <div className="md:col-span-3 text-right mt-4">
                            <button type="submit" className="px-6 py-2 bg-red-600 text-white rounded-lg font-bold shadow-md hover:bg-red-700">Run Assessment</button>
                        </div>
                    </form>
                </Card>
            )}
            <Card title="Recent Triage Entries">
                <div className="overflow-x-auto">
                    <table className="w-full text-left min-w-[800px]">
                        <thead>
                            <tr><th className="p-3">Time</th><th className="p-3">Patient</th><th className="p-3">Severity</th><th className="p-3">Complaint</th><th className="p-3">Assessed By</th><th className="p-3">Status</th></tr>
                        </thead>
                        <tbody>
                            {triageEntries.map(t => (
                                <tr key={t.id} className="border-b hover:bg-gray-50">
                                    <td className="p-3 text-sm text-gray-500">{getUtcDate(t.assessedAt).toLocaleString()}</td>
                                    <td className="p-3 font-medium">{t.patientName}</td>
                                    <td className="p-3"><span className={`px-2 py-1 text-xs font-bold rounded-full ${t.severityLevel === 'P1' ? 'bg-red-100 text-red-800' : t.severityLevel === 'P2' ? 'bg-orange-100 text-orange-800' : 'bg-green-100 text-green-800'}`}>{t.severityLevel} - {t.severityLabel}</span></td>
                                    <td className="p-3 text-gray-700">{t.chiefComplaint || 'N/A'}</td>
                                    <td className="p-3 text-gray-500">{t.assessedBy}</td>
                                    <td className="p-3 font-semibold text-gray-600">{t.status}</td>
                                </tr>
                            ))}
                        </tbody>
                    </table>
                </div>
            </Card>
        </div>
    );
};

const EmergencyQueueView = ({ emergencyQueue, setEmergencyQueue, triageEntries, user, fetchQueueData, users, resources }) => {

    const handleEnqueue = async (triageId, patientId) => {
        try {
            await api.enqueuePatient({ triageId, patientId });
            await fetchQueueData();
        } catch (e) { alert("Failed to add to queue."); }
    };

    const handleAssignDoctor = async (id, doctorName) => {
        try {
            await api.updateQueueStatus(id, { assignedDoctor: doctorName });
            await fetchQueueData();
        } catch (e) { alert("Failed to assign doctor."); }
    };

    const handleAssignBed = async (id, bedName) => {
        try {
            await api.updateQueueStatus(id, { assignedBed: bedName });
            await fetchQueueData();
        } catch (e) { alert("Failed to assign bed."); }
    };

    const handleUpdateStatus = async (id, status) => {
        try {
            await api.updateQueueStatus(id, { queueStatus: status });
            await fetchQueueData();
        } catch (e) { alert("Failed to update status."); }
    };

    const handleDischarge = async (id) => {
        try {
            await api.updateQueueStatus(id, { queueStatus: 'Discharged' });
            await fetchQueueData();
        } catch (e) { alert("Failed to discharge."); }
    };

    // Filter triage entries that are waiting and not in queue
    const waitingTriages = triageEntries.filter(t => t.status === 'Waiting' && !emergencyQueue.find(q => q.triageId === t.id));

    return (
        <div className="space-y-6">
            <h1 className="text-2xl font-bold text-gray-800">Live Emergency Queue</h1>

            <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
                <div className="lg:col-span-2 space-y-6">
                    <Card title="Active Queue (Priority Ordered)">
                        <div className="space-y-4">
                            {emergencyQueue.filter(q => q.queueStatus !== 'Discharged').map(q => (
                                <div key={q.id} className={`p-4 border-l-4 rounded-r-lg shadow-sm bg-white ${q.priorityScore === 1 ? 'border-red-500' : q.priorityScore === 2 ? 'border-orange-500' : 'border-yellow-500'} flex justify-between items-center`}>
                                    <div>
                                        <div className="flex items-center gap-3">
                                            <span className="font-bold text-lg">{q.patientName}</span>
                                            <span className={`px-2 py-0.5 text-xs font-bold rounded text-white ${q.priorityScore === 1 ? 'bg-red-500' : q.priorityScore === 2 ? 'bg-orange-500' : 'bg-yellow-500'}`}>P{q.priorityScore}</span>
                                        </div>
                                        <div className="text-sm text-gray-500 mt-1">Wait Time: {Math.floor((Date.now() - getUtcDate(q.enqueuedAt).getTime()) / 60000)} mins</div>
                                        {q.assignedDoctor && <div className="text-sm text-indigo-600 font-semibold mt-1">Assigned: {q.assignedDoctor}</div>}
                                    </div>
                                    <div className="flex flex-col items-end gap-2">
                                        <div className="flex items-center gap-2">
                                            <span className="text-sm font-semibold text-gray-700 bg-gray-100 px-3 py-1 rounded-full">{q.queueStatus}</span>
                                            {['Admin', 'Doctor', 'Nurse'].includes(user.role) && (
                                                <>
                                                    {q.queueStatus === 'Waiting' && <button onClick={() => handleUpdateStatus(q.id, 'In Treatment')} className="text-xs bg-indigo-100 text-indigo-700 px-3 py-1 rounded hover:bg-indigo-200">Start Treatment</button>}
                                                    {q.queueStatus === 'In Treatment' && ['Admin', 'Doctor'].includes(user.role) && <button onClick={() => handleDischarge(q.id)} className="text-xs bg-green-100 text-green-700 px-3 py-1 rounded hover:bg-green-200">Discharge</button>}
                                                </>
                                            )}
                                        </div>
                                        {['Admin', 'Nurse'].includes(user.role) && q.queueStatus !== 'Discharged' && (
                                            <>
                                                <select
                                                    className="text-xs border rounded px-2 py-1 bg-white outline-none focus:ring-1 focus:ring-indigo-500"
                                                    value={q.assignedDoctor || ""}
                                                    onChange={(e) => handleAssignDoctor(q.id, e.target.value)}
                                                >
                                                    <option value="">Assign Dr...</option>
                                                    {users?.filter(u => u.role === 'Doctor').map(d => (
                                                        <option key={d.id} value={d.name}>{d.name}</option>
                                                    ))}
                                                </select>
                                                {q.queueStatus === 'In Treatment' && (
                                                    <select
                                                        className="text-xs border rounded px-2 py-1 bg-white outline-none focus:ring-1 focus:ring-indigo-500"
                                                        value={q.assignedBed || ""}
                                                        onChange={(e) => handleAssignBed(q.id, e.target.value)}
                                                    >
                                                        <option value="">Assign Bed...</option>
                                                        {resources?.filter(r => r.resourceType === 'Bed' && (r.status === 'Available' || r.resourceName === q.assignedBed)).map(b => (
                                                            <option key={b.id} value={b.resourceName}>{b.resourceName}</option>
                                                        ))}
                                                    </select>
                                                )}
                                            </>
                                        )}
                                    </div>
                                </div>
                            ))}
                            {emergencyQueue.filter(q => q.queueStatus !== 'Discharged').length === 0 && <p className="text-gray-500 italic">The active emergency queue is empty.</p>}
                        </div>
                    </Card>

                    <Card title="Discharged Patients History">
                        <div className="space-y-4">
                            {emergencyQueue.filter(q => q.queueStatus === 'Discharged').map(q => (
                                <div key={q.id} className="p-4 border rounded-lg bg-gray-50 flex justify-between items-center">
                                    <div>
                                        <div className="font-bold">{q.patientName}</div>
                                        <div className="text-sm text-gray-500 mt-1">Discharged At: {getUtcDate(q.dischargedAt).toLocaleString()}</div>
                                    </div>
                                    <span className="text-sm font-semibold text-green-700 bg-green-100 px-3 py-1 rounded-full">Discharged</span>
                                </div>
                            ))}
                            {emergencyQueue.filter(q => q.queueStatus === 'Discharged').length === 0 && <p className="text-gray-500 italic">No discharged patients.</p>}
                        </div>
                    </Card>
                </div>

                <div>
                    <Card title="Pending Transfers">
                        <p className="text-xs text-gray-500 mb-4">Triaged patients waiting to be added to the live queue.</p>
                        <div className="space-y-3">
                            {waitingTriages.map(t => (
                                <div key={t.id} className="p-3 border rounded-lg flex justify-between items-center bg-gray-50">
                                    <div>
                                        <p className="font-semibold text-sm">{t.patientName}</p>
                                        <p className="text-xs text-gray-500">{t.severityLevel}</p>
                                    </div>
                                    {['Admin', 'Doctor', 'Nurse'].includes(user.role) && <button onClick={() => handleEnqueue(t.id, t.patientId)} className="text-xs bg-red-100 text-red-700 px-2 py-1 rounded font-semibold hover:bg-red-200">Enqueue</button>}
                                </div>
                            ))}
                            {waitingTriages.length === 0 && <p className="text-xs text-gray-400">No pending transfers.</p>}
                        </div>
                    </Card>
                </div>
            </div>
        </div>
    );
};

const ResourceManagementView = ({ resources, setResources, user, fetchResources }) => {

    const handleToggleStatus = async (res) => {
        const newStatus = res.status === 'Available' ? 'In Use' : 'Available';
        try {
            await api.updateResource(res.id, { status: newStatus });
            await fetchResources();
        } catch (e) { alert("Failed to update resource."); }
    };

    return (
        <div className="space-y-6">
            <h1 className="text-2xl font-bold text-gray-800">Resource Allocation</h1>
            <div className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-4 gap-4">
                {resources.map(r => (
                    <div key={r.id} className="bg-white p-4 rounded-xl shadow-sm border border-gray-100">
                        <div className="flex justify-between items-start mb-2">
                            <h3 className="font-bold text-gray-800 flex items-center gap-2">
                                {r.resourceType === 'Bed' && <svg className="w-5 h-5 text-indigo-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M3 12h18M3 16h18M3 8v8M21 8v8M5 12V8h14v4" /></svg>}
                                {r.resourceType === 'Ventilator' && <svg className="w-5 h-5 text-teal-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M13 10V3L4 14h7v7l9-11h-7z" /></svg>}
                                {r.resourceType === 'OperatingRoom' && <svg className="w-5 h-5 text-red-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 21V5a2 2 0 00-2-2H7a2 2 0 00-2 2v16m14 0h2m-2 0h-5m-9 0H3m2 0h5M9 7h1m-1 4h1m4-4h1m-1 4h1m-5 10v-5a1 1 0 011-1h2a1 1 0 011 1v5m-4 0h4" /></svg>}
                                {r.resourceType === 'Staff' && <svg className="w-5 h-5 text-blue-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M16 7a4 4 0 11-8 0 4 4 0 018 0zM12 14a7 7 0 00-7 7h14a7 7 0 00-7-7z" /></svg>}
                                {r.resourceName}
                            </h3>
                            <span className={`text-xs font-bold px-2 py-1 rounded-full ${r.status === 'Available' ? 'bg-green-100 text-green-700' : r.status === 'In Use' ? 'bg-red-100 text-red-700' : 'bg-gray-200 text-gray-700'}`}>
                                {r.status}
                            </span>
                        </div>
                        <p className="text-sm text-gray-500 mb-2">{r.resourceType} - {r.location}</p>
                        {r.assignedPatientName && <div className="text-sm font-semibold text-indigo-700 bg-indigo-50 px-2 py-1 rounded mb-4">Patient: {r.assignedPatientName}</div>}
                        {['Admin', 'Doctor', 'Nurse'].includes(user.role) && r.resourceType !== 'Bed' && (
                            <button
                                onClick={() => handleToggleStatus(r)}
                                className={`w-full py-1.5 rounded text-sm font-semibold transition ${r.status === 'Available' ? 'bg-gray-100 hover:bg-gray-200 text-gray-800' : 'bg-green-50 hover:bg-green-100 text-green-700'}`}
                            >
                                {r.status === 'Available' ? 'Mark In Use' : 'Release Resource'}
                            </button>
                        )}
                        {r.resourceType === 'Bed' && <p className="text-xs text-gray-400 text-center italic mt-2">Managed via Live Queue</p>}
                    </div>
                ))}
            </div>
        </div>
    );
};

// --- REST OF VIEWS ---
const PatientManagementView = ({ patients, setPatients, onSelectPatient, user }) => { const [searchTerm, setSearchTerm] = useState(''); const [isAdding, setIsAdding] = useState(false); const [newPatient, setNewPatient] = useState({ name: '', dob: '', gender: 'Male', contact: '', email: '', address: '' }); const canRegister = ['Admin', 'Doctor', 'Nurse', 'Receptionist'].includes(user.role); const filteredPatients = useMemo(() => patients.filter(p => p.name.toLowerCase().includes(searchTerm.toLowerCase())), [patients, searchTerm]); const handleRegister = async (e) => { e.preventDefault(); const p = await api.addPatient(newPatient); setPatients(prev => [...prev, p]); setIsAdding(false); setNewPatient({ name: '', dob: '', gender: 'Male', contact: '', email: '', address: '' }); }; return (<div className="space-y-6"> <div className="flex flex-col sm:flex-row justify-between sm:items-center gap-4"> <h1 className="text-2xl font-bold text-gray-800">Patient Records</h1> {canRegister && <button onClick={() => setIsAdding(!isAdding)} className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">{isAdding ? 'Cancel' : '+ Register Patient'}</button>} </div> {isAdding && (<Card title="New Patient Registration"><form onSubmit={handleRegister} className="grid grid-cols-1 md:grid-cols-2 gap-4"><input type="text" placeholder="Full Name" value={newPatient.name} onChange={(e) => setNewPatient({ ...newPatient, name: e.target.value })} className="p-2 border rounded-md" required /><input type="date" value={newPatient.dob} onChange={(e) => setNewPatient({ ...newPatient, dob: e.target.value })} className="p-2 border rounded-md" required /><select value={newPatient.gender} onChange={(e) => setNewPatient({ ...newPatient, gender: e.target.value })} className="p-2 border rounded-md bg-white"><option>Male</option><option>Female</option><option>Other</option></select><input type="text" placeholder="Contact Number" value={newPatient.contact} onChange={(e) => setNewPatient({ ...newPatient, contact: e.target.value })} className="p-2 border rounded-md" required /><input type="email" placeholder="Email Address" value={newPatient.email} onChange={(e) => setNewPatient({ ...newPatient, email: e.target.value })} className="p-2 border rounded-md md:col-span-2" required /><input type="text" placeholder="Address" value={newPatient.address} onChange={(e) => setNewPatient({ ...newPatient, address: e.target.value })} className="p-2 border rounded-md md:col-span-2" required /><div className="md:col-span-2 text-right"><button type="submit" className="px-4 py-2 bg-green-500 text-white rounded-lg hover:bg-green-600">Save Patient</button></div></form></Card>)}<Card><div className="mb-4"> <input type="text" placeholder="Search by patient name..." value={searchTerm} onChange={(e) => setSearchTerm(e.target.value)} className="w-full p-3 border rounded-lg focus:ring-2 focus:ring-indigo-500 outline-none" /> </div><div className="overflow-x-auto"><table className="w-full text-left min-w-[600px]"><thead><tr><th className="p-3">Name</th><th className="p-3">Date of Birth</th><th className="p-3">Contact</th><th className="p-3"></th></tr></thead><tbody>{filteredPatients.map(p => (<tr key={p.id} className="border-b hover:bg-gray-50"><td className="p-3 font-medium">{p.name}</td><td className="p-3 text-gray-600">{new Date(p.dob).toLocaleDateString()}</td><td className="p-3 text-gray-600">{p.contact}</td><td className="p-3 text-right"><button onClick={() => onSelectPatient(p)} className="text-indigo-600 text-sm font-semibold hover:underline">View Details</button></td></tr>))}</tbody></table></div></Card></div>); };
const UserManagementView = ({ users, setUsers }) => { const [isAdding, setIsAdding] = useState(false); const [newUser, setNewUser] = useState({ name: '', username: '', password: '', role: 'Receptionist' }); const handleRegister = async (e) => { e.preventDefault(); try { const result = await api.registerUser(newUser.username, newUser.password, newUser.role, newUser.name); if (result.success) { setUsers(prev => [...prev, result.user]); setIsAdding(false); setNewUser({ name: '', username: '', password: '', role: 'Receptionist' }); } } catch (err) { alert("Failed to create user."); } }; return (<div className="space-y-6"> <div className="flex justify-between items-center"><h1 className="text-2xl font-bold text-gray-800">User Management</h1><button onClick={() => setIsAdding(!isAdding)} className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">{isAdding ? 'Cancel' : '+ Add New User'}</button></div> {isAdding && (<Card title="Create New User Account"><form onSubmit={handleRegister} className="grid grid-cols-1 md:grid-cols-2 gap-4"><input type="text" placeholder="Full Name" value={newUser.name} onChange={(e) => setNewUser({ ...newUser, name: e.target.value })} className="p-2 border rounded-md" required /><input type="text" placeholder="Username" value={newUser.username} onChange={(e) => setNewUser({ ...newUser, username: e.target.value })} className="p-2 border rounded-md" required /><input type="password" placeholder="Password" value={newUser.password} onChange={(e) => setNewUser({ ...newUser, password: e.target.value })} className="p-2 border rounded-md" required /><select value={newUser.role} onChange={(e) => setNewUser({ ...newUser, role: e.target.value })} className="p-2 border rounded-md bg-white"><option>Receptionist</option><option>Nurse</option><option>Doctor</option><option>Admin</option></select><div className="md:col-span-2 text-right"><button type="submit" className="px-4 py-2 bg-green-500 text-white rounded-lg hover:bg-green-600">Create User</button></div></form></Card>)}<Card title="All System Users"><div className="overflow-x-auto"><table className="w-full text-left min-w-[500px]"><thead><tr><th className="p-3">Name</th><th className="p-3">Username</th><th className="p-3">Role</th></tr></thead><tbody>{users.map(u => (<tr key={u.id} className="border-b hover:bg-gray-50"><td className="p-3 font-medium">{u.name}</td><td className="p-3 text-gray-600">{u.username}</td><td className="p-3 text-gray-600">{u.role}</td></tr>))}</tbody></table></div></Card></div>); };
const AppointmentView = ({ appointments, setAppointments, patients, users }) => { const [isScheduling, setIsScheduling] = useState(false); const [newAppointment, setNewAppointment] = useState({ patientId: '', doctorName: '', date: '', reason: '' }); const doctors = users.filter(u => u.role === 'Doctor'); const handleSchedule = async (e) => { e.preventDefault(); const p = patients.find(p => p.id === parseInt(newAppointment.patientId)); if (!p) return; const result = await api.addAppointment({ ...newAppointment, patientId: parseInt(newAppointment.patientId), date: newAppointment.date.replace('T', ' '), patientName: p.name }); setAppointments(prev => [...prev, result]); setIsScheduling(false); setNewAppointment({ patientId: '', doctorName: '', date: '', reason: '' }); }; return (<div className="space-y-6"> <div className="flex justify-between items-center"><h1 className="text-2xl font-bold text-gray-800">Appointment Management</h1><button onClick={() => setIsScheduling(!isScheduling)} className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">{isScheduling ? 'Cancel' : '+ Schedule Appointment'}</button></div> {isScheduling && (<Card title="Schedule New Appointment"><form onSubmit={handleSchedule} className="grid grid-cols-1 md:grid-cols-2 gap-4"><select value={newAppointment.patientId} onChange={(e) => setNewAppointment({ ...newAppointment, patientId: e.target.value })} className="p-2 border rounded-md bg-white" required><option value="">Select Patient</option>{patients.map(p => <option key={p.id} value={p.id}>{p.name}</option>)}</select><select value={newAppointment.doctorName} onChange={(e) => setNewAppointment({ ...newAppointment, doctorName: e.target.value })} className="p-2 border rounded-md bg-white" required><option value="">Select Doctor</option>{doctors.map(d => <option key={d.id} value={d.name}>{d.name}</option>)}</select><input type="datetime-local" value={newAppointment.date} onChange={(e) => setNewAppointment({ ...newAppointment, date: e.target.value })} className="p-2 border rounded-md" required /><input type="text" placeholder="Reason for visit" value={newAppointment.reason} onChange={(e) => setNewAppointment({ ...newAppointment, reason: e.target.value })} className="p-2 border rounded-md" required /><div className="md:col-span-2 text-right"><button type="submit" className="px-4 py-2 bg-green-500 text-white rounded-lg hover:bg-green-600">Schedule</button></div></form></Card>)}<Card title="All Appointments"><div className="overflow-x-auto"><table className="w-full text-left min-w-[600px]"><thead><tr><th className="p-3">Patient</th><th className="p-3">Doctor</th><th className="p-3">Date & Time</th><th className="p-3">Status</th></tr></thead><tbody>{appointments.map(app => (<tr key={app.id} className="border-b hover:bg-gray-50"><td className="p-3 font-medium">{app.patientName}</td><td className="p-3 text-gray-600">{app.doctorName}</td><td className="p-3 text-gray-600">{getUtcDate(app.date).toLocaleString()}</td><td className="p-3"><span className="px-2 py-1 text-xs font-semibold text-blue-800 bg-blue-100 rounded-full">{app.status}</span></td></tr>))}</tbody></table></div></Card></div>); };
const PatientDetailView = ({ patient, records, onBack, onUpdate, onDelete, onAddRecord, user }) => { const [isEditing, setIsEditing] = useState(false); const [editedPatient, setEditedPatient] = useState(patient); const [newRecord, setNewRecord] = useState({ diagnosis: '', prescription: '', notes: '' }); const canEditPatient = ['Admin', 'Doctor'].includes(user.role); const canAddRecord = user.role === 'Doctor'; const canDeletePatient = user.role === 'Admin'; const handleSave = async () => { await onUpdate(editedPatient); setIsEditing(false); }; const handleDelete = () => { if (confirm(`Are you sure you want to delete patient ${patient.name}?`)) onDelete(patient.id); }; const handleAddRecord = (e) => { e.preventDefault(); onAddRecord({ ...newRecord, patientId: patient.id, doctorName: user.name, date: new Date().toISOString().split('T')[0] }); setNewRecord({ diagnosis: '', prescription: '', notes: '' }); }; return (<div className="space-y-6"> <button onClick={onBack} className="text-indigo-600 font-semibold hover:underline">&larr; Back to Patient List</button> <Card title={isEditing ? 'Edit Patient Information' : `Patient Profile: ${patient.name}`} actions={(<div className="flex items-center gap-2">{canEditPatient && !isEditing && <button onClick={() => setIsEditing(true)} className="px-4 py-2 bg-indigo-100 text-indigo-700 rounded-lg text-sm font-semibold hover:bg-indigo-200">Edit</button>}{isEditing && <button onClick={handleSave} className="px-4 py-2 bg-green-500 text-white rounded-lg text-sm font-semibold hover:bg-green-600">Save</button>}{isEditing && <button onClick={() => setIsEditing(false)} className="px-4 py-2 bg-gray-200 text-gray-700 rounded-lg text-sm font-semibold hover:bg-gray-300">Cancel</button>}{canDeletePatient && !isEditing && <button onClick={handleDelete} className="px-4 py-2 bg-red-100 text-red-700 rounded-lg text-sm font-semibold hover:bg-red-200">Delete</button>}</div>)}><div className="grid grid-cols-1 md:grid-cols-2 gap-x-8 gap-y-4 text-gray-600">{Object.entries({ Name: 'name', 'Date of Birth': 'dob', Gender: 'gender', Contact: 'contact', Email: 'email', Address: 'address' }).map(([label, key]) => (<div key={key} className={key === 'address' ? 'md:col-span-2' : ''}><strong>{label}:</strong> {isEditing ? <input type={key === 'dob' ? 'date' : 'text'} defaultValue={patient[key]} onChange={(e) => setEditedPatient({ ...editedPatient, [key]: e.target.value })} className="p-1 border rounded-md w-full" /> : (key === 'dob' ? new Date(patient[key]).toLocaleDateString() : patient[key])}</div>))}</div></Card><Card title="Medical History"><div className="space-y-4">{records.length > 0 ? records.slice().reverse().map(rec => (<div key={rec.id} className="p-4 bg-gray-50 rounded-lg border"><p className="font-bold text-gray-800">{rec.diagnosis}</p><p className="text-sm text-gray-500">Recorded by {rec.doctorName} on {new Date(rec.date).toLocaleDateString()}</p><p className="mt-2 text-gray-700"><strong>Prescription:</strong> {rec.prescription}</p><p className="mt-1 text-gray-700"><strong>Notes:</strong> {rec.notes}</p></div>)) : <p className="text-gray-500">No medical records found.</p>}</div></Card>{canAddRecord && (<Card title="Add New Medical Record"><form onSubmit={handleAddRecord} className="space-y-4"><input type="text" placeholder="Diagnosis" value={newRecord.diagnosis} onChange={(e) => setNewRecord({ ...newRecord, diagnosis: e.target.value })} className="w-full p-2 border rounded-md" required /><input type="text" placeholder="Prescription" value={newRecord.prescription} onChange={(e) => setNewRecord({ ...newRecord, prescription: e.target.value })} className="w-full p-2 border rounded-md" required /><textarea placeholder="Notes" value={newRecord.notes} onChange={(e) => setNewRecord({ ...newRecord, notes: e.target.value })} className="w-full p-2 border rounded-md h-24"></textarea><div className="text-right"><button type="submit" className="px-5 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">Add Record</button></div></form></Card>)}</div>); };


// --- Main App Layout ---

const PatientDetailWrapper = ({ patients, medicalRecords, user, onUpdate, onDelete, onAddRecord }) => {
    const { id } = useParams();
    const navigate = useNavigate();
    const patient = patients.find(p => p.id === parseInt(id));
    if (!patient) return <div className="p-8 text-center text-gray-500">Patient not found</div>;
    return <PatientDetailView patient={patient} records={medicalRecords.filter(r => r.patientId === patient.id)} onBack={() => navigate('/patients')} user={user} onUpdate={onUpdate} onDelete={onDelete} onAddRecord={onAddRecord} />;
};

const AppLayout = ({ user, onLogout }) => {
    const [isSidebarOpen, setIsSidebarOpen] = useState(false);
    const [loading, setLoading] = useState(true);
    const navigate = useNavigate();

    // Core data
    const [patients, setPatients] = useState([]);
    const [medicalRecords, setMedicalRecords] = useState([]);
    const [appointments, setAppointments] = useState([]);
    const [users, setUsers] = useState([]);

    // Emergency Data
    const [triageEntries, setTriageEntries] = useState([]);
    const [emergencyQueue, setEmergencyQueue] = useState([]);
    const [resources, setResources] = useState([]);

    const fetchData = async () => {
        try {
            setLoading(true);
            const [pData, rData, aData, uData, trData, qData, resData] = await Promise.all([
                api.getPatients(), api.getMedicalRecords(), api.getAppointments(), api.getUsers(),
                api.getTriageEntries(), api.getEmergencyQueue(), api.getResources()
            ]);
            setPatients(pData); setMedicalRecords(rData); setAppointments(aData); setUsers(uData);
            setTriageEntries(trData); setEmergencyQueue(qData); setResources(resData);
        } catch (error) {
            console.error("Failed to fetch initial data:", error);
            // alert("Could not connect to the backend server.");
        } finally { setLoading(false); }
    };

    const fetchQueueData = async () => {
        const [trData, qData, resData] = await Promise.all([
            api.getTriageEntries(), 
            api.getEmergencyQueue(),
            api.getResources()
        ]);
        setTriageEntries(trData); 
        setEmergencyQueue(qData);
        setResources(resData);
    };

    const fetchResourcesData = async () => {
        const resData = await api.getResources();
        setResources(resData);
    };

    useEffect(() => { fetchData(); }, []);

    const handleUpdatePatient = async (p) => { try { const res = await api.updatePatient(p); setPatients(patients.map(pt => pt.id === res.id ? res : pt)); } catch (e) { alert("Could not update patient."); } };
    const handleDeletePatient = async (id) => { try { await api.deletePatient(id); setPatients(patients.filter(p => p.id !== id)); navigate('/patients'); } catch (e) { alert("Could not delete patient."); } };
    const handleAddRecord = async (rec) => { try { const res = await api.addMedicalRecord(rec); setMedicalRecords(prev => [...prev, res]); } catch (e) { alert("Could not add record.") } };

    const now = new Date();
    const isToday = (dateString) => {
        const d = getUtcDate(dateString);
        return d.getFullYear() === now.getFullYear() && d.getMonth() === now.getMonth() && d.getDate() === now.getDate();
    };

    const dashboardStats = {
        totalPatients: patients.length,
        appointmentsToday: appointments.filter(a => isToday(a.date)).length,
        totalStaff: users.length,
        appointments: appointments.filter(a => a.status === 'Scheduled' && isToday(a.date)),
        activeQueueCount: emergencyQueue.filter(q => q.queueStatus !== 'Discharged').length,
        criticalCount: emergencyQueue.filter(q => q.priorityScore === 1 && q.queueStatus !== 'Discharged').length,
        resourcesInUse: resources.filter(r => r.status === 'In Use').length,
        emergencyQueue: emergencyQueue.filter(q => q.queueStatus !== 'Discharged')
    };

    const NavLink = ({ to, children, requiredRoles, icon }) => {
        const location = useLocation();
        if (requiredRoles && !requiredRoles.includes(user.role)) return null;
        const isActive = location.pathname.startsWith(to) && (to !== '/' || location.pathname === '/dashboard');
        return (
            <Link to={to} onClick={() => setIsSidebarOpen(false)}
                className={`w-full text-left px-4 py-3 rounded-lg text-sm font-medium transition-colors duration-200 flex items-center gap-3 ${isActive ? 'bg-red-600 text-white shadow-md' : 'text-gray-700 hover:bg-red-50 hover:text-red-700'}`}>
                {icon}
                {children}
            </Link>
        );
    };

    const renderRoutes = () => {
        if (loading) return <LoadingSpinner fullScreen={true} />;
        return (
            <Routes>
                <Route path="/" element={<Navigate to="/dashboard" replace />} />
                <Route path="/dashboard" element={<DashboardView stats={dashboardStats} />} />
                <Route path="/triage" element={<TriageView triageEntries={triageEntries} setTriageEntries={setTriageEntries} patients={patients} user={user} />} />
                <Route path="/queue" element={<EmergencyQueueView emergencyQueue={emergencyQueue} setEmergencyQueue={setEmergencyQueue} triageEntries={triageEntries} user={user} fetchQueueData={fetchQueueData} users={users} resources={resources} />} />
                <Route path="/resources" element={<ResourceManagementView resources={resources} setResources={setResources} user={user} fetchResources={fetchResourcesData} />} />
                <Route path="/patients" element={<PatientManagementView patients={patients} setPatients={setPatients} onSelectPatient={(p) => navigate(`/patients/${p.id}`)} user={user} />} />
                <Route path="/patients/:id" element={<PatientDetailWrapper patients={patients} medicalRecords={medicalRecords} user={user} onUpdate={handleUpdatePatient} onDelete={handleDeletePatient} onAddRecord={handleAddRecord} />} />
                <Route path="/users" element={<UserManagementView users={users} setUsers={setUsers} />} />
                <Route path="/appointments" element={<AppointmentView appointments={appointments} setAppointments={setAppointments} patients={patients} users={users} />} />
                <Route path="*" element={<Navigate to="/dashboard" replace />} />
            </Routes>
        );
    };

    return (
        <div className="relative min-h-screen md:flex bg-gray-50">
            {/* Mobile Header */}
            <div className="bg-red-600 text-white flex justify-between items-center md:hidden p-4 shadow-md">
                <span className="font-bold text-lg">EmergiCare</span>
                <button onClick={() => setIsSidebarOpen(!isSidebarOpen)} className="focus:outline-none">
                    <svg className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 6h16M4 12h16M4 18h16" /></svg>
                </button>
            </div>

            <aside className={`bg-white shadow-xl w-64 min-h-screen space-y-6 py-7 flex flex-col absolute inset-y-0 left-0 transform ${isSidebarOpen ? "translate-x-0" : "-translate-x-full"} md:relative md:translate-x-0 transition duration-200 ease-in-out z-20`}>
                <div className="px-6 text-center border-b pb-6">
                    <h1 className="text-2xl font-extrabold text-red-600 tracking-tight flex items-center justify-center gap-2">
                        <svg className="w-6 h-6" fill="currentColor" viewBox="0 0 20 20"><path fillRule="evenodd" d="M3.172 5.172a4 4 0 015.656 0L10 6.343l1.172-1.171a4 4 0 115.656 5.656L10 17.657l-6.828-6.829a4 4 0 010-5.656z" clipRule="evenodd" /></svg>
                        EmergiCare
                    </h1>
                </div>

                <div className="px-6 border-b pb-4">
                    <p className="text-sm font-semibold text-gray-800">{user.name}</p>
                    <p className="text-xs font-bold text-red-600 bg-red-50 inline-block px-2 py-1 rounded-md mt-1">{user.role}</p>
                </div>

                <nav className="flex-grow px-4 space-y-1">
                    <NavLink to="/dashboard" requiredRoles={['Admin', 'Doctor', 'Nurse', 'Receptionist']} icon={<svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2V6zM14 6a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2V6zM4 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2H6a2 2 0 01-2-2v-2zM14 16a2 2 0 012-2h2a2 2 0 012 2v2a2 2 0 01-2 2h-2a2 2 0 01-2-2v-2z" /></svg>}>Dashboard</NavLink>
                    <div className="pt-4 pb-2"><p className="text-xs font-bold text-gray-400 uppercase tracking-wider px-2">Emergency Core</p></div>
                    <NavLink to="/triage" requiredRoles={['Admin', 'Doctor', 'Nurse']} icon={<svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 5H7a2 2 0 00-2 2v12a2 2 0 002 2h10a2 2 0 002-2V7a2 2 0 00-2-2h-2M9 5a2 2 0 002 2h2a2 2 0 002-2M9 5a2 2 0 012-2h2a2 2 0 012 2m-3 7h3m-3 4h3m-6-4h.01M9 16h.01" /></svg>}>Triage Assessment</NavLink>
                    <NavLink to="/queue" requiredRoles={['Admin', 'Doctor', 'Nurse', 'Receptionist']} icon={<svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 8v4l3 3m6-3a9 9 0 11-18 0 9 9 0 0118 0z" /></svg>}>Live Queue</NavLink>
                    <NavLink to="/resources" requiredRoles={['Admin', 'Doctor', 'Nurse']} icon={<svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 11H5m14 0a2 2 0 012 2v6a2 2 0 01-2 2H5a2 2 0 01-2-2v-6a2 2 0 012-2m14 0V9a2 2 0 00-2-2M5 11V9a2 2 0 012-2m0 0V5a2 2 0 012-2h6a2 2 0 012 2v2M7 7h10" /></svg>}>Resources</NavLink>

                    <div className="pt-4 pb-2"><p className="text-xs font-bold text-gray-400 uppercase tracking-wider px-2">General</p></div>
                    <NavLink to="/patients" requiredRoles={['Admin', 'Doctor', 'Nurse', 'Receptionist']} icon={<svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0z" /></svg>}>Patient Records</NavLink>
                    <NavLink to="/appointments" requiredRoles={['Admin', 'Doctor', 'Nurse', 'Receptionist']} icon={<svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 7V3m8 4V3m-9 8h10M5 21h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v12a2 2 0 002 2z" /></svg>}>Appointments</NavLink>
                    <NavLink to="/users" requiredRoles={['Admin']} icon={<svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 4.354a4 4 0 110 5.292M15 21H3v-1a6 6 0 0112 0v1zm0 0h6v-1a6 6 0 00-9-5.197M15 21v-1a6 6 0 00-1.781-4.121M12 11c-3.333 0-6 2.686-6 6v1h12v-1c0-3.314-2.667-6-6-6z" /></svg>}>Staff</NavLink>
                </nav>

                <div className="p-4 border-t">
                    <button onClick={onLogout} className="w-full text-left px-4 py-2 rounded-lg text-sm font-bold text-gray-500 hover:bg-gray-100 flex items-center gap-2">
                        <svg className="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M17 16l4-4m0 0l-4-4m4 4H7m6 4v1a3 3 0 01-3 3H6a3 3 0 01-3-3V7a3 3 0 013-3h4a3 3 0 013 3v1" /></svg>
                        Sign Out
                    </button>
                </div>
            </aside>

            <main className="flex-1 p-4 md:p-8 overflow-y-auto h-screen">
                {renderRoutes()}
            </main>
        </div>
    );
}

export default function App() {
    return (
        <Router>
            <AuthScreen />
        </Router>
    );
}
