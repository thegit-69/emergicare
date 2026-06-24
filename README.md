# Healthcare Cloud Data Management System

A modern cloud-based healthcare data management system built for secure patient records, appointment scheduling, and medical record management. This is a Cloud Computing Capstone Project.

## 🎯 Project Overview

A full-stack healthcare application with:
- ✅ **PostgreSQL** database for local development
- ✅ **Express.js** REST API backend with bcrypt authentication
- ✅ **React** frontend with role-based access control (Admin, Doctor, Receptionist)
- ✅ **Docker Compose** for multi-container orchestration
- ✅ **Supabase** for production database
- ✅ **Azure App Service** for deployment

## 🚀 Quick Start

### Prerequisites
- Node.js 18+
- Docker Desktop (optional)
- PostgreSQL 15 (for local development)

### Option 1: Docker Compose (Recommended)
```bash
docker-compose up --build
# Frontend: http://localhost:8080
# Backend:  http://localhost:3001
```

### Option 2: Local Development
```bash
# Install PostgreSQL 15 (see POSTGRES_SETUP_GUIDE.md)

# Backend
cd backend && npm install && npm start

# Frontend (new terminal)
cd healthcare-frontend && npm run dev
```

### Test Login
- **Username:** `admin`
- **Password:** `Admin@123`

## 📋 Features

### Authentication & Authorization
- Secure login with bcrypt password hashing (10 salt rounds)
- Role-based access control (RBAC):
  - **Admin**: Full system access
  - **Doctor**: Patient records, medical records, appointments
  - **Receptionist**: Appointment scheduling, patient registration

### Patient Management
- Register and manage patient profiles
- Track patient demographics and contact information
- View patient medical history

### Appointments
- Schedule and manage appointments
- Filter by date and status
- Real-time appointment status updates

### Medical Records
- Document diagnosis and treatment
- Store prescriptions and medical notes
- Secure access based on user role

## 🏗️ Architecture

### Technology Stack
| Component | Technology | Version |
|-----------|-----------|---------|
| Database | PostgreSQL | 15.x |
| Backend | Express.js | 4.19.2 |
| Runtime | Node.js | 18-alpine |
| Frontend | React | 19.1.1 |
| Build Tool | Vite | 7.1.7 |
| Styling | Tailwind CSS | 4.1.14 |
| Auth | Bcrypt | 5.1.1 |
| Containerization | Docker | Latest |

### Database Schema

```sql
Users (id, username, password, role, name)
Patients (id, name, dob, gender, contact, email, address)
Appointments (id, patientId, patientName, doctorName, date, reason, status)
MedicalRecords (id, patientId, doctorName, date, diagnosis, prescription, notes)
```

### API Endpoints

#### Authentication
- `POST /api/login` - User login with credentials
- `POST /api/users` - Create new user (admin only)

#### Users
- `GET /api/users` - List all users

#### Patients
- `GET /api/patients` - List all patients
- `POST /api/patients` - Register new patient
- `PUT /api/patients/:id` - Update patient info
- `DELETE /api/patients/:id` - Delete patient record

#### Appointments
- `GET /api/appointments` - List all appointments
- `POST /api/appointments` - Schedule appointment

#### Medical Records
- `GET /api/medical-records` - List medical records
- `POST /api/medical-records` - Add medical record

## 📂 Project Structure

```
healthcare-system/
├── backend/                    # Express.js API server
│   ├── server.js              # PostgreSQL version
│   ├── package.json           # Dependencies (uses 'pg')
│   ├── .env                   # Environment config
│   ├── .env.example           # Config template
│   └── Dockerfile             # Backend container
├── healthcare-frontend/        # React frontend
│   ├── src/
│   │   ├── App.jsx            # Main app component
│   │   ├── main.jsx
│   │   └── index.css
│   ├── vite.config.js
│   ├── package.json
│   └── Dockerfile             # Frontend container
├── init-db.sql               # PostgreSQL schema
├── docker-compose.yml        # Multi-container config
├── POSTGRES_SETUP_GUIDE.md   # Detailed PostgreSQL setup
├── COMPLETE_SETUP.md         # Full setup instructions
└── README.md                 # This file
```

## 🔧 Setup Instructions

### 1. Local PostgreSQL Setup (Windows 11)

See **[POSTGRES_SETUP_GUIDE.md](./POSTGRES_SETUP_GUIDE.md)** for detailed instructions.

Quick version:
```bash
# Install PostgreSQL 15
# Create database
psql -U postgres -c "CREATE DATABASE healthcare_db;"

# Initialize schema
psql -U postgres -d healthcare_db -f init-db.sql

# Update backend/.env
# Set DB_HOST=localhost, DB_USER=postgres, DB_PASSWORD=<your-password>
```

### 2. Backend Setup

```bash
cd backend
npm install
npm start
# Runs on http://localhost:3001
```

### 3. Frontend Setup

```bash
cd healthcare-frontend
npm install
npm run dev
# Runs on http://localhost:5173
```

### 4. Using Docker Compose

```bash
docker-compose up --build
# All services start automatically
# Database initializes automatically
```

## 🔐 Security Features

- ✅ **Bcrypt Password Hashing**: 10 salt rounds
- ✅ **Role-Based Access Control**: User-specific features
- ✅ **CORS Protection**: Cross-origin requests controlled
- ✅ **SSL/TLS Ready**: For production deployment
- ✅ **Environment Variables**: Sensitive data not committed

## 📦 Environment Configuration

### Development (Local PostgreSQL)
```ini
DB_HOST=localhost
DB_USER=postgres
DB_PASSWORD=
DB_NAME=healthcare_db
DB_PORT=5432
```

### Docker Development
```ini
DB_HOST=postgres
DB_PORT=5432
DB_NAME=healthcare_db
```

### Production (Supabase)
```ini
DB_HOST=your-project.postgres.supabase.co
DB_PORT=5432
DB_NAME=postgres
DB_USER=postgres
DB_PASSWORD=<secure-password>
```

See **[COMPLETE_SETUP.md](./COMPLETE_SETUP.md)** for detailed configuration guide.

## 🚀 Deployment

### Azure App Service

1. **Frontend**: Deploy to Azure Static Web Apps or App Service
2. **Backend**: Deploy to Azure App Service
3. **Database**: Use Supabase managed PostgreSQL

See separate deployment guide for Azure configuration.

### Docker Deployment

```bash
# Build and run with Docker Compose
docker-compose up --build

# Or individually
docker build -t healthcare-backend ./backend
docker build -t healthcare-frontend ./healthcare-frontend
```

## 📝 Sample User Credentials

| Role | Username | Password |
|------|----------|----------|
| Admin | admin | Admin@123 |
| Doctor | doctor1 | Doctor@123 |
| Receptionist | receptionist1 | Receptionist@123 |

**Note**: Sample credentials are seeded on first database initialization. In production, create users through the `/api/users` endpoint.

## 🔄 Database Migration from MySQL to PostgreSQL

This project has been migrated from MySQL to PostgreSQL:

- ✅ Backend rewritten to use `pg` driver instead of `mysql2`
- ✅ SQL syntax updated for PostgreSQL compatibility
- ✅ Docker Compose includes PostgreSQL container
- ✅ Connection pooling configured for production load
- ✅ All queries use parameterized queries ($1, $2, etc.)

**No additional MySQL dependencies required.**

## 📊 Database Features

- ✅ **Foreign Key Constraints**: Referential integrity maintained
- ✅ **Cascading Deletes**: Automatic cleanup of related records
- ✅ **Indexes**: Performance optimized for common queries
- ✅ **Connection Pooling**: Efficient resource utilization
- ✅ **SSL Support**: Encrypted connections for production

## 🧪 Testing

### Login API
```bash
curl -X POST http://localhost:3001/api/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"Admin@123"}'
```

### Create New User
```bash
curl -X POST http://localhost:3001/api/login \
  -H "Content-Type: application/json" \
  -d '{
    "username":"newdoctor",
    "password":"NewPass@123",
    "role":"Doctor",
    "name":"Dr. New User"
  }'
```

### Get All Patients
```bash
curl http://localhost:3001/api/patients
```

## 🐛 Troubleshooting

### Connection Refused (Port 5432)
- Ensure PostgreSQL is running: `psql -U postgres -c "\l"`
- Check database exists: `psql -U postgres -l | grep healthcare_db`
- Restart PostgreSQL service if needed

### Docker Containers Not Starting
- Clean volumes: `docker-compose down -v`
- Rebuild: `docker-compose up --build`
- Check logs: `docker-compose logs -f`

### Frontend Cannot Reach Backend
- Verify backend is running: `curl http://localhost:3001`
- Check `.env` API endpoint configuration
- Enable CORS in backend (enabled by default)

For more troubleshooting, see **[COMPLETE_SETUP.md](./COMPLETE_SETUP.md)**.

## 📚 Documentation

- **[POSTGRES_SETUP_GUIDE.md](./POSTGRES_SETUP_GUIDE.md)** - PostgreSQL installation & configuration
- **[COMPLETE_SETUP.md](./COMPLETE_SETUP.md)** - Full project setup guide
- **[Express API Docs](https://expressjs.com/)** - Backend framework
- **[React Docs](https://react.dev/)** - Frontend framework
- **[PostgreSQL Docs](https://www.postgresql.org/docs/)** - Database documentation

## 🔄 Future Enhancements

- [ ] JWT token-based authentication
- [ ] Email notifications for appointments
- [ ] Mobile app (React Native)
- [ ] Advanced reporting and analytics
- [ ] AI-powered diagnosis suggestions
- [ ] Multi-language support
- [ ] Audit logging for compliance

## 🤝 Contributing

This is a capstone project. For questions or improvements:
1. Document your changes
2. Test thoroughly
3. Update relevant documentation

## 📄 License

This project is created for educational purposes as part of a Cloud Computing Capstone.

## 👨‍💼 Project Status

**Current Phase**: Development with PostgreSQL ✅
- PostgreSQL migration complete
- Docker Compose setup complete
- Ready for local development testing
- Production deployment pending

**Next Phase**: Production deployment to Azure + Supabase

---

**Questions or Issues?** Check the troubleshooting section or see [COMPLETE_SETUP.md](./COMPLETE_SETUP.md)


