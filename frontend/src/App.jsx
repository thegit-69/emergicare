import React, { useState, useEffect, useMemo } from 'react';
import axios from 'axios';

// --- REAL API SETUP ---
// For local setup uncomment the below line
// const API_URL = 'http://localhost:3001/api';
const API_URL = import.meta.env.VITE_API_BASE_URL;

// Helper to decode JWT payload safely on client side
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

// Initialize global Axios Authorization header if token exists in localStorage
const initialToken = localStorage.getItem('healthcare_token');
if (initialToken) {
    axios.defaults.headers.common['Authorization'] = `Bearer ${initialToken}`;
}

const api = {
    login: async (username, password) => { const response = await axios.post(`${API_URL}/login`, { username, password }); return response.data; },
    registerUser: async (username, password, role, name) => { const response = await axios.post(`${API_URL}/users`, { username, password, role, name }); return { success: true, user: response.data }; },
    addAppointment: async (appointment) => { const response = await axios.post(`${API_URL}/appointments`, appointment); return response.data; },
    getUsers: async () => { const response = await axios.get(`${API_URL}/users`); return response.data; },
    getPatients: async () => { const response = await axios.get(`${API_URL}/patients`); return response.data; },
    getMedicalRecords: async () => { const response = await axios.get(`${API_URL}/medical-records`); return response.data; },
    getAppointments: async () => { const response = await axios.get(`${API_URL}/appointments`); return response.data; },
    addPatient: async (patient) => { const response = await axios.post(`${API_URL}/patients`, patient); return response.data; },
    addMedicalRecord: async (record) => { const response = await axios.post(`${API_URL}/medical-records`, record); return response.data; },
    updatePatient: async (patient) => { const response = await axios.put(`${API_URL}/patients/${patient.id}`, patient); return response.data; },
    deletePatient: async (patientId) => { const response = await axios.delete(`${API_URL}/patients/${patientId}`); return response.data; },
};
// --- END REAL API ---


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
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-indigo-600"></div>
    </div>
);


// --- VIEWS (Complete implementations) ---
const AuthScreen = () => {
    const [user, setUser] = useState(() => {
        const token = localStorage.getItem('healthcare_token');
        if (token) {
            const decoded = parseJwt(token);
            // Check if token is expired
            if (decoded && decoded.exp * 1000 > Date.now()) {
                return decoded;
            } else {
                localStorage.removeItem('healthcare_token');
            }
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
                <div className="max-w-md w-full bg-white p-8 rounded-xl shadow-lg">
                    <div className="text-center mb-8"><h1 className="text-3xl font-bold text-indigo-600">Mediflow</h1><h3 className="text-3xm font-bold text-indigo-600">Hospital Management system</h3><p className="text-gray-500">Please sign in to continue</p></div>
                    {error && <p className="bg-red-100 text-red-700 p-3 rounded-md mb-4 text-sm">{error}</p>}
                    <form onSubmit={handleLogin} className="space-y-4">
                        <input type="text" placeholder="Username" value={username} onChange={(e) => setUsername(e.target.value)} className="w-full p-3 border rounded-md focus:ring-2 focus:ring-indigo-500 outline-none" required />
                        <input type="password" placeholder="Password" value={password} onChange={(e) => setPassword(e.target.value)} className="w-full p-3 border rounded-md focus:ring-2 focus:ring-indigo-500 outline-none" required />
                        <SubmitButton isSubmitting={isSubmitting} text="Login" loadingText="Signing in..." className="w-full py-3 px-4 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700 transition duration-300 font-semibold" />
                    </form>
                </div>
            </div>
        );
    };

    if (user) return <AppLayout user={user} onLogout={() => {
        localStorage.removeItem('healthcare_token');
        setUser(null);
    }} />;
    return <LoginComponent />;
};

const DashboardView = ({ stats }) => (<div className="space-y-6"> <h1 className="text-2xl font-bold text-gray-800">Dashboard</h1> <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-6"> <StatCard label="Total Patients" value={stats.totalPatients} color="bg-blue-100" icon={<svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6 text-blue-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M17 20h5v-2a3 3 0 00-5.356-1.857M17 20H7m10 0v-2c0-.656-.126-1.283-.356-1.857M7 20H2v-2a3 3 0 015.356-1.857M7 20v-2c0-.656.126-1.283.356-1.857m0 0a5.002 5.002 0 019.288 0M15 7a3 3 0 11-6 0 3 3 0 016 0z" /></svg>} /> <StatCard label="Appointments Today" value={stats.appointmentsToday} color="bg-green-100" icon={<svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6 text-green-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 7V3m8 4V3m-9 8h10M5 21h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v12a2 2 0 002 2z" /></svg>} /> <StatCard label="Total Staff" value={stats.totalStaff} color="bg-indigo-100" icon={<svg xmlns="http://www.w3.org/2000/svg" className="h-6 w-6 text-indigo-500" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 4.354a4 4 0 110 5.292M15 21H3v-1a6 6 0 0112 0v1zm0 0h6v-1a6 6 0 00-9-5.197M15 21v-1a6 6 0 00-1.781-4.121M12 11c-3.333 0-6 2.686-6 6v1h12v-1c0-3.314-2.667-6-6-6z" /></svg>} /> </div> <Card title="Upcoming Appointments"> <div className="overflow-x-auto"> <table className="w-full text-left min-w-[600px]"> <thead> <tr> <th className="p-3 text-sm font-semibold text-gray-600 border-b">Patient</th> <th className="p-3 text-sm font-semibold text-gray-600 border-b">Doctor</th> <th className="p-3 text-sm font-semibold text-gray-600 border-b">Time</th> <th className="p-3 text-sm font-semibold text-gray-600 border-b">Reason</th> </tr> </thead> <tbody> {stats.appointments.map(app => (<tr key={app.id} className="border-b hover:bg-gray-50"> <td className="p-3 font-medium">{app.patientName}</td> <td className="p-3 text-gray-600">{app.doctorName}</td> <td className="p-3 text-gray-600">{new Date(app.date).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}</td> <td className="p-3 text-gray-600">{app.reason}</td> </tr>))} </tbody> </table> </div> </Card> </div>);
const PatientDetailView = ({ patient, records, onBack, onUpdate, onDelete, onAddRecord, user }) => { const [isEditing, setIsEditing] = useState(false); const [editedPatient, setEditedPatient] = useState(patient); const [newRecord, setNewRecord] = useState({ diagnosis: '', prescription: '', notes: '' }); const canEditPatient = ['Admin', 'Doctor'].includes(user.role); const canAddRecord = user.role === 'Doctor'; const canDeletePatient = user.role === 'Admin'; const handleSave = async () => { await onUpdate(editedPatient); setIsEditing(false); }; const handleDelete = () => { if (confirm(`Are you sure you want to delete patient ${patient.name}?`)) onDelete(patient.id); }; const handleAddRecord = (e) => { e.preventDefault(); onAddRecord({ ...newRecord, patientId: patient.id, doctorName: user.name, date: new Date().toISOString().split('T')[0] }); setNewRecord({ diagnosis: '', prescription: '', notes: '' }); }; return (<div className="space-y-6"> <button onClick={onBack} className="text-indigo-600 font-semibold hover:underline">&larr; Back to Patient List</button> <Card title={isEditing ? 'Edit Patient Information' : `Patient Profile: ${patient.name}`} actions={(<div className="flex items-center gap-2">{canEditPatient && !isEditing && <button onClick={() => setIsEditing(true)} className="px-4 py-2 bg-indigo-100 text-indigo-700 rounded-lg text-sm font-semibold hover:bg-indigo-200">Edit</button>}{isEditing && <button onClick={handleSave} className="px-4 py-2 bg-green-500 text-white rounded-lg text-sm font-semibold hover:bg-green-600">Save</button>}{isEditing && <button onClick={() => setIsEditing(false)} className="px-4 py-2 bg-gray-200 text-gray-700 rounded-lg text-sm font-semibold hover:bg-gray-300">Cancel</button>}{canDeletePatient && !isEditing && <button onClick={handleDelete} className="px-4 py-2 bg-red-100 text-red-700 rounded-lg text-sm font-semibold hover:bg-red-200">Delete</button>}</div>)}><div className="grid grid-cols-1 md:grid-cols-2 gap-x-8 gap-y-4 text-gray-600">{Object.entries({ Name: 'name', 'Date of Birth': 'dob', Gender: 'gender', Contact: 'contact', Email: 'email', Address: 'address' }).map(([label, key]) => (<div key={key} className={key === 'address' ? 'md:col-span-2' : ''}><strong>{label}:</strong> {isEditing ? <input type={key === 'dob' ? 'date' : 'text'} defaultValue={patient[key]} onChange={(e) => setEditedPatient({ ...editedPatient, [key]: e.target.value })} className="p-1 border rounded-md w-full" /> : (key === 'dob' ? new Date(patient[key]).toLocaleDateString() : patient[key])}</div>))}</div></Card><Card title="Medical History"><div className="space-y-4">{records.length > 0 ? records.slice().reverse().map(rec => (<div key={rec.id} className="p-4 bg-gray-50 rounded-lg border"><p className="font-bold text-gray-800">{rec.diagnosis}</p><p className="text-sm text-gray-500">Recorded by {rec.doctorName} on {new Date(rec.date).toLocaleDateString()}</p><p className="mt-2 text-gray-700"><strong>Prescription:</strong> {rec.prescription}</p><p className="mt-1 text-gray-700"><strong>Notes:</strong> {rec.notes}</p></div>)) : <p className="text-gray-500">No medical records found.</p>}</div></Card>{canAddRecord && (<Card title="Add New Medical Record"><form onSubmit={handleAddRecord} className="space-y-4"><input type="text" placeholder="Diagnosis" value={newRecord.diagnosis} onChange={(e) => setNewRecord({ ...newRecord, diagnosis: e.target.value })} className="w-full p-2 border rounded-md" required /><input type="text" placeholder="Prescription" value={newRecord.prescription} onChange={(e) => setNewRecord({ ...newRecord, prescription: e.target.value })} className="w-full p-2 border rounded-md" required /><textarea placeholder="Notes" value={newRecord.notes} onChange={(e) => setNewRecord({ ...newRecord, notes: e.target.value })} className="w-full p-2 border rounded-md h-24"></textarea><div className="text-right"><button type="submit" className="px-5 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">Add Record</button></div></form></Card>)}</div>); };
const PatientManagementView = ({ patients, setPatients, onSelectPatient, user }) => { const [searchTerm, setSearchTerm] = useState(''); const [isAdding, setIsAdding] = useState(false); const [newPatient, setNewPatient] = useState({ name: '', dob: '', gender: 'Male', contact: '', email: '', address: '' }); const canRegister = ['Admin', 'Doctor', 'Receptionist'].includes(user.role); const filteredPatients = useMemo(() => patients.filter(p => p.name.toLowerCase().includes(searchTerm.toLowerCase())), [patients, searchTerm]); const handleRegister = async (e) => { e.preventDefault(); const p = await api.addPatient(newPatient); setPatients(prev => [...prev, p]); setIsAdding(false); setNewPatient({ name: '', dob: '', gender: 'Male', contact: '', email: '', address: '' }); }; return (<div className="space-y-6"> <div className="flex flex-col sm:flex-row justify-between sm:items-center gap-4"> <h1 className="text-2xl font-bold text-gray-800">Patient Records</h1> {canRegister && <button onClick={() => setIsAdding(!isAdding)} className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">{isAdding ? 'Cancel' : '+ Register Patient'}</button>} </div> {isAdding && (<Card title="New Patient Registration"><form onSubmit={handleRegister} className="grid grid-cols-1 md:grid-cols-2 gap-4"><input type="text" placeholder="Full Name" value={newPatient.name} onChange={(e) => setNewPatient({ ...newPatient, name: e.target.value })} className="p-2 border rounded-md" required /><input type="date" value={newPatient.dob} onChange={(e) => setNewPatient({ ...newPatient, dob: e.target.value })} className="p-2 border rounded-md" required /><select value={newPatient.gender} onChange={(e) => setNewPatient({ ...newPatient, gender: e.target.value })} className="p-2 border rounded-md bg-white"><option>Male</option><option>Female</option><option>Other</option></select><input type="text" placeholder="Contact Number" value={newPatient.contact} onChange={(e) => setNewPatient({ ...newPatient, contact: e.target.value })} className="p-2 border rounded-md" required /><input type="email" placeholder="Email Address" value={newPatient.email} onChange={(e) => setNewPatient({ ...newPatient, email: e.target.value })} className="p-2 border rounded-md md:col-span-2" required /><input type="text" placeholder="Address" value={newPatient.address} onChange={(e) => setNewPatient({ ...newPatient, address: e.target.value })} className="p-2 border rounded-md md:col-span-2" required /><div className="md:col-span-2 text-right"><button type="submit" className="px-4 py-2 bg-green-500 text-white rounded-lg hover:bg-green-600">Save Patient</button></div></form></Card>)}<Card><div className="mb-4"> <input type="text" placeholder="Search by patient name..." value={searchTerm} onChange={(e) => setSearchTerm(e.target.value)} className="w-full p-3 border rounded-lg focus:ring-2 focus:ring-indigo-500 outline-none" /> </div><div className="overflow-x-auto"><table className="w-full text-left min-w-[600px]"><thead><tr><th className="p-3">Name</th><th className="p-3">Date of Birth</th><th className="p-3">Contact</th><th className="p-3"></th></tr></thead><tbody>{filteredPatients.map(p => (<tr key={p.id} className="border-b hover:bg-gray-50"><td className="p-3 font-medium">{p.name}</td><td className="p-3 text-gray-600">{new Date(p.dob).toLocaleDateString()}</td><td className="p-3 text-gray-600">{p.contact}</td><td className="p-3 text-right"><button onClick={() => onSelectPatient(p)} className="text-indigo-600 text-sm font-semibold hover:underline">View Details</button></td></tr>))}</tbody></table></div></Card></div>); };
const UserManagementView = ({ users, setUsers }) => { const [isAdding, setIsAdding] = useState(false); const [newUser, setNewUser] = useState({ name: '', username: '', password: '', role: 'Receptionist' }); const handleRegister = async (e) => { e.preventDefault(); try { const result = await api.registerUser(newUser.username, newUser.password, newUser.role, newUser.name); if (result.success) { setUsers(prev => [...prev, result.user]); setIsAdding(false); setNewUser({ name: '', username: '', password: '', role: 'Receptionist' }); } } catch (err) { alert("Failed to create user."); } }; return (<div className="space-y-6"> <div className="flex justify-between items-center"><h1 className="text-2xl font-bold text-gray-800">User Management</h1><button onClick={() => setIsAdding(!isAdding)} className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">{isAdding ? 'Cancel' : '+ Add New User'}</button></div> {isAdding && (<Card title="Create New User Account"><form onSubmit={handleRegister} className="grid grid-cols-1 md:grid-cols-2 gap-4"><input type="text" placeholder="Full Name" value={newUser.name} onChange={(e) => setNewUser({ ...newUser, name: e.target.value })} className="p-2 border rounded-md" required /><input type="text" placeholder="Username" value={newUser.username} onChange={(e) => setNewUser({ ...newUser, username: e.target.value })} className="p-2 border rounded-md" required /><input type="password" placeholder="Password" value={newUser.password} onChange={(e) => setNewUser({ ...newUser, password: e.target.value })} className="p-2 border rounded-md" required /><select value={newUser.role} onChange={(e) => setNewUser({ ...newUser, role: e.target.value })} className="p-2 border rounded-md bg-white"><option>Receptionist</option><option>Doctor</option><option>Admin</option></select><div className="md:col-span-2 text-right"><button type="submit" className="px-4 py-2 bg-green-500 text-white rounded-lg hover:bg-green-600">Create User</button></div></form></Card>)}<Card title="All System Users"><div className="overflow-x-auto"><table className="w-full text-left min-w-[500px]"><thead><tr><th className="p-3">Name</th><th className="p-3">Username</th><th className="p-3">Role</th></tr></thead><tbody>{users.map(u => (<tr key={u.id} className="border-b hover:bg-gray-50"><td className="p-3 font-medium">{u.name}</td><td className="p-3 text-gray-600">{u.username}</td><td className="p-3 text-gray-600">{u.role}</td></tr>))}</tbody></table></div></Card></div>); };
const AppointmentView = ({ appointments, setAppointments, patients, users }) => { const [isScheduling, setIsScheduling] = useState(false); const [newAppointment, setNewAppointment] = useState({ patientId: '', doctorName: '', date: '', reason: '' }); const doctors = users.filter(u => u.role === 'Doctor'); const handleSchedule = async (e) => { e.preventDefault(); const p = patients.find(p => p.id === parseInt(newAppointment.patientId)); if (!p) return; const result = await api.addAppointment({ ...newAppointment, date: newAppointment.date.replace('T', ' '), patientName: p.name }); setAppointments(prev => [...prev, result]); setIsScheduling(false); setNewAppointment({ patientId: '', doctorName: '', date: '', reason: '' }); }; return (<div className="space-y-6"> <div className="flex justify-between items-center"><h1 className="text-2xl font-bold text-gray-800">Appointment Management</h1><button onClick={() => setIsScheduling(!isScheduling)} className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700">{isScheduling ? 'Cancel' : '+ Schedule Appointment'}</button></div> {isScheduling && (<Card title="Schedule New Appointment"><form onSubmit={handleSchedule} className="grid grid-cols-1 md:grid-cols-2 gap-4"><select value={newAppointment.patientId} onChange={(e) => setNewAppointment({ ...newAppointment, patientId: e.target.value })} className="p-2 border rounded-md bg-white" required><option value="">Select Patient</option>{patients.map(p => <option key={p.id} value={p.id}>{p.name}</option>)}</select><select value={newAppointment.doctorName} onChange={(e) => setNewAppointment({ ...newAppointment, doctorName: e.target.value })} className="p-2 border rounded-md bg-white" required><option value="">Select Doctor</option>{doctors.map(d => <option key={d.id} value={d.name}>{d.name}</option>)}</select><input type="datetime-local" value={newAppointment.date} onChange={(e) => setNewAppointment({ ...newAppointment, date: e.target.value })} className="p-2 border rounded-md" required /><input type="text" placeholder="Reason for visit" value={newAppointment.reason} onChange={(e) => setNewAppointment({ ...newAppointment, reason: e.target.value })} className="p-2 border rounded-md" required /><div className="md:col-span-2 text-right"><button type="submit" className="px-4 py-2 bg-green-500 text-white rounded-lg hover:bg-green-600">Schedule</button></div></form></Card>)}<Card title="All Appointments"><div className="overflow-x-auto"><table className="w-full text-left min-w-[600px]"><thead><tr><th className="p-3">Patient</th><th className="p-3">Doctor</th><th className="p-3">Date & Time</th><th className="p-3">Status</th></tr></thead><tbody>{appointments.map(app => (<tr key={app.id} className="border-b hover:bg-gray-50"><td className="p-3 font-medium">{app.patientName}</td><td className="p-3 text-gray-600">{app.doctorName}</td><td className="p-3 text-gray-600">{new Date(app.date).toLocaleString()}</td><td className="p-3"><span className="px-2 py-1 text-xs font-semibold text-blue-800 bg-blue-100 rounded-full">{app.status}</span></td></tr>))}</tbody></table></div></Card></div>); };


// --- Main App Layout ---
const AppLayout = ({ user, onLogout }) => {
    const [isSidebarOpen, setIsSidebarOpen] = useState(false);
    const [activeView, setActiveView] = useState('dashboard');
    const [selectedPatient, setSelectedPatient] = useState(null);
    const [loading, setLoading] = useState(true);
    const [patients, setPatients] = useState([]);
    const [medicalRecords, setMedicalRecords] = useState([]);
    const [appointments, setAppointments] = useState([]);
    const [users, setUsers] = useState([]);

    useEffect(() => {
        const fetchData = async () => {
            try {
                setLoading(true);
                const [pData, rData, aData, uData] = await Promise.all([api.getPatients(), api.getMedicalRecords(), api.getAppointments(), api.getUsers()]);
                setPatients(pData); setMedicalRecords(rData); setAppointments(aData); setUsers(uData);
            } catch (error) {
                console.error("Failed to fetch initial data:", error); alert("Could not connect to the backend server.");
            } finally { setLoading(false); }
        };
        fetchData();
    }, []);

    const handleUpdatePatient = async (p) => { try { const res = await api.updatePatient(p); setPatients(patients.map(pt => pt.id === res.id ? res : pt)); setSelectedPatient(res); } catch (e) { alert("Could not update patient."); } };
    const handleDeletePatient = async (id) => { try { await api.deletePatient(id); setPatients(patients.filter(p => p.id !== id)); setSelectedPatient(null); setActiveView('patients'); } catch (e) { alert("Could not delete patient."); } };
    const handleAddRecord = async (rec) => { try { const res = await api.addMedicalRecord(rec); setMedicalRecords(prev => [...prev, res]); } catch (e) { alert("Could not add record.") } };

    const today = new Date().toISOString().split('T')[0];
    const dashboardStats = { totalPatients: patients.length, appointmentsToday: appointments.filter(a => a.date.startsWith(today)).length, totalStaff: users.length, appointments: appointments.filter(a => a.status === 'Scheduled' && a.date.startsWith(today)), };

    const NavLink = ({ viewName, children, requiredRoles }) => { if (requiredRoles && !requiredRoles.includes(user.role)) return null; return <button onClick={() => { setActiveView(viewName); setSelectedPatient(null); setIsSidebarOpen(false); }} className={`w-full text-left px-4 py-2 rounded-lg text-sm font-medium transition-colors duration-200 ${activeView === viewName ? 'bg-indigo-600 text-white border border-indigo-700 shadow-md' : 'text-gray-700 hover:bg-indigo-100 hover:text-indigo-700'}`}>{children}</button>; };

    const renderView = () => {
        if (loading) return <LoadingSpinner fullScreen={true} />;
        if (selectedPatient) return <PatientDetailView patient={selectedPatient} records={medicalRecords.filter(r => r.patientId === selectedPatient.id)} onBack={() => setSelectedPatient(null)} user={user} onUpdate={handleUpdatePatient} onDelete={handleDeletePatient} onAddRecord={handleAddRecord} />;
        switch (activeView) {
            case 'patients': return <PatientManagementView patients={patients} setPatients={setPatients} onSelectPatient={setSelectedPatient} user={user} />;
            case 'users': return <UserManagementView users={users} setUsers={setUsers} />;
            case 'appointments': return <AppointmentView appointments={appointments} setAppointments={setAppointments} patients={patients} users={users} />;
            case 'dashboard': default: return <DashboardView stats={dashboardStats} />;
        }
    };

    return (
        <div className="relative min-h-screen md:flex">
            {/* Mobile Header */}
            <div className="bg-white text-gray-800 flex justify-between md:hidden">
                <a href="#" className="block p-4 text-white font-bold"></a>
                <button onClick={() => setIsSidebarOpen(!isSidebarOpen)} className="p-4 focus:outline-none focus:bg-gray-700">
                    <svg className="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor"><path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M4 6h16M4 12h16M4 18h16" /></svg>
                </button>
            </div>

            {/* --- FINAL, VERIFIED SIDEBAR --- */}
            <aside className={`bg-white shadow-xl w-64 min-h-screen space-y-6 py-7 px-2 absolute inset-y-0 left-0 transform ${isSidebarOpen ? "translate-x-0" : "-translate-x-full"} 
                             md:relative md:translate-x-0 transition duration-200 ease-in-out
                             md:flex md:flex-col`}>

                <div className="p-4 text-center border-b">
                    <h1 className="text-xl font-bold text-indigo-600">Mediflow</h1>
                    <h3 className="text-xm font-bold text-indigo-500">Health Care Cloud Management System</h3>
                </div>

                <div className="p-4 border-b">
                    <p className="text-sm font-semibold text-gray-700">Welcome, {user.name}</p>
                    <p className="text-xs text-gray-500 bg-gray-100 inline-block px-2 py-1 rounded-full mt-1">{user.role}</p>
                </div>

                <nav className="flex-grow p-4 space-y-2">
                    <NavLink viewName="dashboard" requiredRoles={['Admin', 'Doctor', 'Receptionist']}>Dashboard</NavLink>
                    <NavLink viewName="appointments" requiredRoles={['Admin', 'Doctor', 'Receptionist']}>Appointments</NavLink>
                    <NavLink viewName="patients" requiredRoles={['Admin', 'Doctor', 'Receptionist']}>Patient Records</NavLink>
                    <NavLink viewName="users" requiredRoles={['Admin']}>User Management</NavLink>
                </nav>

                <div className="p-4 border-t">
                    <button onClick={onLogout} className="w-full text-left px-4 py-2 rounded-lg text-sm font-medium text-red-600 hover:bg-red-100 hover:text-red-800">Logout</button>
                </div>
            </aside>

            <main className="flex-1 p-4 md:p-8">
                {renderView()}
            </main>
        </div>
    );
}

export default function App() {
    return <AuthScreen />;
}

