#pragma once

namespace FileSystem
{
	/// <summary>Représente un fichier en mémoire (HANDLE, taille et data)</summary>
	typedef struct
	{
		HANDLE file;
		SIZE_T size;
		LPBYTE data;
	}memory_file_t;

	/// <summary>Combine un chemin (répertoire) et une fichier en un chemin d'accès</summary>
	/// <param name="dest">Buffer de sortie</param>
	/// <param name="dir">Chemin du dossier à combiner</param>
	/// <param name="file">Nom du fichier à combiner</param>
	/// <returns>FALSE en cas d'échec, TRUE sinon</returns>
	bool combinePath(LPWSTR dest, const LPWSTR dir, const LPWSTR file);

	/// <summary>Créer les dossiers non existants dans le chemin fourni</summary>
	/// <param name="path">Chemin à créer</param>
	/// <param name="securityAttributes">Atributs de sécurité avec lesquels créer les dossier</param>
	/// <returns>FALSE en cas d'échec, TRUE sinon</returns>
	bool createDirectoryTree(LPWSTR path, LPSECURITY_ATTRIBUTES securityAttributes);

	/// <summary>Lis le fichier dont le chemin est passé en paramètre et le place en mémoire</summary>
	/// <param name="fileName">Chemin complet vers le fichier à lire</param>
	/// <param name="memoryFile">Pointeur vers la structure de sortie</param>
	/// <returns>FALSE en cas d'échec, TRUE sinon</returns>
	bool fileToMemory(LPWSTR filePath, memory_file_t *memoryFile);

	/// <summary>Supprime le fichier dont le chemin est passé en paramètre</summary>
	/// <param name="filePath">Chemin complet vers le fichier à supprimer</param>
	/// <returns>FALSE en cas d'échec, TRUE sinon</returns>
	bool deleteFile(LPWSTR filePath);

	/// <summary>Enregistre les données passées en paramètre dans le fichier passé en paramètre</summary>
	/// <param name="filePath">Chemin complet vers le fichier dans lequel enregistrer les données</param>
	/// <param name="data">Pointeur vers les données à enregistrer</param>
	/// <param name="dataSize">Taille des données à écrire</param>
	/// <returns>FALSE en cas d'échec, TRUE sinon</returns>
	bool saveDataToFile(const LPWSTR filePath, const void * data, DWORD dataSize);

	/// <summary>Ferme le fichier mémoire passé en paramètre</summary>
	/// <param name="memoryFile">Pointeur vers le fichier mémoire à fermer</param>
	void closeMemoryFile(memory_file_t *memoryFile);
}