/*
Copyright (C) 2013 Victor Luchits

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef SYS_THREADS_H
#define SYS_THREADS_H

int Sys_Thread_Create( qthread_t **pthread, void *(*routine) (void*), void *param );
void Sys_Thread_Join( qthread_t *thread );

int Sys_Mutex_Create( qmutex_t **pmutex );
void Sys_Mutex_Destroy( qmutex_t *mutex );
void Sys_Mutex_Lock( qmutex_t *mutex );
void Sys_Mutex_Unlock( qmutex_t *mutex );

#endif // SYS_THREADS_H
